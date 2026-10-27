import pytest
from app.adapters.c_engine_adapter import CEngineAdapter


def test_c_engine_adapter_structure():
    """Verify CEngineAdapter interface contract and method presence."""
    assert hasattr(CEngineAdapter, "get_binary_path")
    assert hasattr(CEngineAdapter, "is_available")
    assert hasattr(CEngineAdapter, "execute_probe")

    # If compiled executable exists, check path integrity
    bin_path = CEngineAdapter.get_binary_path()
    if bin_path:
        assert bin_path.endswith("netpulse_engine.exe") or bin_path.endswith("netpulse_engine")


@pytest.mark.asyncio
async def test_c_engine_invalid_host():
    """Verify C engine adapter rejects malformed hostnames before executing."""
    res = await CEngineAdapter.execute_probe(
        mode="tcp",
        host="bad$$host##invalid",
        port=80,
    )
    assert res is None
