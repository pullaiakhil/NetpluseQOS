from typing import List
from pydantic_settings import BaseSettings, SettingsConfigDict


class Settings(BaseSettings):
    """Application configuration settings."""

    APP_NAME: str = "NetPulse QoS"
    APP_VERSION: str = "1.0.0"
    DEBUG: bool = False
    
    # Database
    DATABASE_URL: str = "sqlite+aiosqlite:///./netpulse.db"
    
    # CORS
    CORS_ORIGINS: List[str] = [
        "http://localhost:5173",
        "http://127.0.0.1:5173",
    ]

    # Probing Defaults
    DEFAULT_PROBE_INTERVAL_SECONDS: float = 5.0
    PROBES_PER_QOS_SNAPSHOT: int = 5
    DEFAULT_TIMEOUT_SECONDS: float = 3.0
    DEFAULT_CONSECUTIVE_VIOLATIONS: int = 3
    DEFAULT_AUTO_RESOLVE_COUNT: int = 3

    model_config = SettingsConfigDict(
        env_file=".env",
        env_file_encoding="utf-8",
        extra="ignore"
    )


settings = Settings()
