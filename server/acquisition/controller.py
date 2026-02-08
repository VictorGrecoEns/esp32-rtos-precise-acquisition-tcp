from time import time
import numpy as np

from rich.console import Console
from rich.table import Table
from rich.live import Live
from rich.progress import Progress, BarColumn, TextColumn, TimeElapsedColumn

from acquisition.storage import DataStorage


class AcquisitionController:
    def __init__(self, n_blocks: int):
        self.n_blocks = n_blocks
        self.received = 0
        self.last_timestamp = None
        self.delay = 0.0

        self.storage: DataStorage

        self.console = Console()
        self.live: Live | None = None

    def init(self, fe: int, storage: DataStorage):
        self.received = 0
        self.last_timestamp = None
        self.storage = storage
        self.fe = fe
        self.delay = time()

        # --- Tableau ---
        self.table = Table(title="Acquisition en cours", expand=True)
        self.table.add_column("Reçu", justify="right")
        self.table.add_column("Total", justify="right")
        self.table.add_column("Moyenne éch V", justify="right")
        self.table.add_column("Délai récep (ms)", justify="right")

        # --- Progress bar ---
        self.progress = Progress(
            TextColumn("[bold blue]Progression"),
            BarColumn(),
            TextColumn("{task.completed}/{task.total} blocs"),
            TimeElapsedColumn(),
            expand=True,
        )
        self.task_id = self.progress.add_task("acq", total=self.n_blocks)

        # --- Layout Live ---
        self.live = Live(
            self._render(),
            console=self.console,
            refresh_per_second=10,
        )
        self.live.__enter__()

    def _render(self):
        from rich.layout import Layout

        layout = Layout()
        layout.split_column(
            Layout(self.table, ratio=3),
            Layout(self.progress, ratio=1),
        )
        return layout

    def process_block(self, timestamp: int, samples: list[int]):
        self.received += 1
        mean_v = np.mean(samples)
        delay_ms = self.delay * 1e3

        self.table.add_row(
            f"{self.received}",
            f"{self.n_blocks}",
            f"{mean_v:.2f}",
            f"{delay_ms:.2f}",
        )

        self.progress.update(self.task_id, advance=1)

        self.storage.append_block(samples)

        self.last_timestamp = timestamp
        self.delay = time()

    def is_finished(self) -> bool:
        return self.received >= self.n_blocks

    def close(self):
        if self.live:
            self.live.__exit__(None, None, None)
        if self.storage:
            self.storage.close()
