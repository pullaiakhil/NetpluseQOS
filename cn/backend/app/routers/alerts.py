from datetime import datetime, timezone
from typing import List, Optional
from fastapi import APIRouter, Depends, HTTPException, Query, status
from sqlalchemy import desc, select
from sqlalchemy.ext.asyncio import AsyncSession

from app.database import get_db
from app.models.models import Alert
from app.schemas.schemas import (
    AlertAcknowledgeResponse,
    AlertResolveResponse,
    AlertResponse,
)

router = APIRouter(prefix="/alerts", tags=["Alerts"])


@router.get("", response_model=List[AlertResponse])
async def list_alerts(
    session_id: Optional[int] = Query(default=None, description="Filter by session ID"),
    severity: Optional[str] = Query(default=None, description="Filter by severity (info, warning, critical)"),
    acknowledged: Optional[bool] = Query(default=None, description="Filter by acknowledged status"),
    resolved: Optional[bool] = Query(default=None, description="Filter by resolved status"),
    skip: int = 0,
    limit: int = 50,
    db: AsyncSession = Depends(get_db),
):
    """Retrieve list of SLA threshold alerts with optional filtering."""
    stmt = select(Alert)
    if session_id is not None:
        stmt = stmt.where(Alert.session_id == session_id)
    if severity is not None:
        stmt = stmt.where(Alert.severity == severity)
    if acknowledged is not None:
        stmt = stmt.where(Alert.acknowledged == acknowledged)
    if resolved is True:
        stmt = stmt.where(Alert.resolved_at.is_not(None))
    elif resolved is False:
        stmt = stmt.where(Alert.resolved_at.is_(None))

    stmt = stmt.order_by(desc(Alert.timestamp)).offset(skip).limit(limit)
    res = await db.execute(stmt)
    return res.scalars().all()


@router.put("/{alert_id}/acknowledge", response_model=AlertAcknowledgeResponse)
async def acknowledge_alert(
    alert_id: int,
    db: AsyncSession = Depends(get_db),
):
    """Mark an active alert as acknowledged."""
    stmt = select(Alert).where(Alert.id == alert_id)
    res = await db.execute(stmt)
    alert = res.scalar_one_or_none()
    if not alert:
        raise HTTPException(
            status_code=status.HTTP_404_NOT_FOUND,
            detail=f"Alert with id {alert_id} not found",
        )

    alert.acknowledged = True
    await db.commit()
    return AlertAcknowledgeResponse(
        id=alert.id,
        acknowledged=True,
        message="Alert acknowledged successfully",
    )


@router.put("/{alert_id}/resolve", response_model=AlertResolveResponse)
async def resolve_alert(
    alert_id: int,
    db: AsyncSession = Depends(get_db),
):
    """Manually resolve an active alert."""
    stmt = select(Alert).where(Alert.id == alert_id)
    res = await db.execute(stmt)
    alert = res.scalar_one_or_none()
    if not alert:
        raise HTTPException(
            status_code=status.HTTP_404_NOT_FOUND,
            detail=f"Alert with id {alert_id} not found",
        )

    now_utc = datetime.now(timezone.utc)
    alert.resolved_at = now_utc
    await db.commit()
    return AlertResolveResponse(
        id=alert.id,
        resolved_at=now_utc,
        message="Alert resolved successfully",
    )
