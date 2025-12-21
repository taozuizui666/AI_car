# plot_lidar_scatter_custom_scale.py
import numpy as np
import matplotlib.pyplot as plt
from matplotlib.widgets import Button, TextBox
import sys
from pathlib import Path
import re

# --------------- Configure area --------------
FILENAME = r"D:\data_cleaned2.txt"
NUM_POINTS = 200
pause_seconds = 0.001
r_min = 0
r_max = 2500
POINT_SIZE = 5

# ===== Highlight configuration =====
HIGHLIGHT_INDICES = [  1,  23,  24, 151, 176, 177, 179, 180, 181, 182, 184, 185, 188, 192, 194, 195, 196, 197, 198, 199]
HIGHLIGHT_COLOR = "red"
HIGHLIGHT_SIZE = 10
# ==================================

save_frames = False
out_dir = "frames"
# ----------------------------------


def read_all_lines(fname):
    rows = []
    p = Path(fname)
    if not p.exists():
        raise FileNotFoundError(f"{fname} not found.")
    with p.open("r", encoding="utf-8", errors="ignore") as f:
        for lineno, raw in enumerate(f, start=1):
            line = raw.strip()
            if not line:
                continue
            parts = [s.strip() for s in line.split(",") if s.strip()]
            vals = parts[:NUM_POINTS]
            if len(vals) < NUM_POINTS:
                vals += ["0"] * (NUM_POINTS - len(vals))
            try:
                arr = np.array([float(v) for v in vals], dtype=float)
            except ValueError:
                print(f"Warning: line {lineno} invalid, filled with zeros", file=sys.stderr)
                arr = np.zeros(NUM_POINTS)
            rows.append(arr)
    return rows


class LidarPlayer:
    def __init__(
        self,
        data_rows,
        angles,
        use_r_min,
        use_r_max,
        pause_seconds,
        save_frames=False,
        out_dir="frames",
    ):
        self.data_rows = data_rows
        self.base_angles = angles.copy()   # 原始角度
        self.angles = angles.copy()        # 当前显示角度
        self.use_r_min = use_r_min
        self.use_r_max = use_r_max
        self.pause_seconds = pause_seconds
        self.save_frames = save_frames
        self.out_dir = out_dir

        self.total = len(data_rows)
        self.idx = 0
        self.paused = False
        self._ignore_textbox_callback = False

        # ---------- Figure ----------
        self.fig, self.ax = plt.subplots(
            figsize=(6, 6), subplot_kw=dict(projection="polar")
        )
        plt.subplots_adjust(bottom=0.28, top=0.88)
        self.ax.set_ylim(self.use_r_min, self.use_r_max)

        r0 = np.where(data_rows[0] == 0.0, np.nan, data_rows[0])

        self.sc = self.ax.scatter(self.angles, r0, s=POINT_SIZE, alpha=0.75)

        self.highlight_indices = [
            i for i in HIGHLIGHT_INDICES if 0 <= i < len(self.angles)
        ]
        self.sc_highlight = self.ax.scatter(
            self.angles[self.highlight_indices],
            r0[self.highlight_indices],
            s=HIGHLIGHT_SIZE,
            c=HIGHLIGHT_COLOR,
            alpha=0.9,
        )

        # ---------- TextBox ----------
        ax_text = plt.axes([0.25, 0.90, 0.50, 0.05])
        self.textbox = TextBox(
            ax_text, "Frame (cur/total):", initial=f"{self.idx + 1}/{self.total}"
        )
        self.textbox.on_submit(self._on_text_submit)

        # ---------- Buttons ----------
        ax_pause = plt.axes([0.40, 0.17, 0.12, 0.06])
        ax_prev = plt.axes([0.18, 0.17, 0.12, 0.06])
        ax_next = plt.axes([0.62, 0.17, 0.12, 0.06])

        self.btn_pause = Button(ax_pause, "Pause")
        self.btn_prev = Button(ax_prev, "Prev")
        self.btn_next = Button(ax_next, "Next")

        self.btn_pause.on_clicked(self._on_pause_clicked)
        self.btn_prev.on_clicked(self._on_prev_clicked)
        self.btn_next.on_clicked(self._on_next_clicked)

        # ---- New transform buttons ----
        ax_rl = plt.axes([0.05, 0.05, 0.18, 0.06])
        ax_rr = plt.axes([0.28, 0.05, 0.18, 0.06])
        ax_mlr = plt.axes([0.51, 0.05, 0.18, 0.06])
        ax_mfb = plt.axes([0.74, 0.05, 0.18, 0.06])

        self.btn_rot_l = Button(ax_rl, "Rotate +90°")
        self.btn_rot_r = Button(ax_rr, "Rotate -90°")
        self.btn_mirror_lr = Button(ax_mlr, "Mirror L/R")
        self.btn_mirror_fb = Button(ax_mfb, "Mirror F/B")

        self.btn_rot_l.on_clicked(self._rotate_left)
        self.btn_rot_r.on_clicked(self._rotate_right)
        self.btn_mirror_lr.on_clicked(self._mirror_lr)
        self.btn_mirror_fb.on_clicked(self._mirror_fb)

        # ---------- Timer ----------
        interval_ms = max(10, int(self.pause_seconds * 1000))
        self.timer = self.fig.canvas.new_timer(interval=interval_ms)
        self.timer.add_callback(self._on_timer)
        self.timer.start()

        self._update_plot(initial=True)

    # ---------- Transform operations ----------
    def _rotate_left(self, event):
        self.angles = (self.angles + np.pi / 2) % (2 * np.pi)
        self._update_plot()

    def _rotate_right(self, event):
        self.angles = (self.angles - np.pi / 2) % (2 * np.pi)
        self._update_plot()

    def _mirror_lr(self, event):
        # 左右镜像（θ -> π - θ）
        self.angles = (np.pi - self.angles) % (2 * np.pi)
        self._update_plot()

    def _mirror_fb(self, event):
        # 前后镜像（θ -> -θ）
        self.angles = (-self.angles) % (2 * np.pi)
        self._update_plot()

    # ---------- Playback ----------
    def _on_pause_clicked(self, event):
        self.paused = not self.paused
        self.btn_pause.label.set_text("Play" if self.paused else "Pause")

    def _on_prev_clicked(self, event):
        self.paused = True
        self.btn_pause.label.set_text("Play")
        self.idx = (self.idx - 1) % self.total
        self._update_plot()

    def _on_next_clicked(self, event):
        self.paused = True
        self.btn_pause.label.set_text("Play")
        self.idx = (self.idx + 1) % self.total
        self._update_plot()

    def _on_text_submit(self, text):
        if self._ignore_textbox_callback:
            return
        m = re.search(r"(\d+)", text)
        if not m:
            self._set_textbox_value(self.idx + 1)
            return
        val = max(1, min(int(m.group(1)), self.total))
        self.idx = val - 1
        self.paused = True
        self.btn_pause.label.set_text("Play")
        self._update_plot()

    def _on_timer(self):
        if not self.paused:
            self.idx = (self.idx + 1) % self.total
            self._update_plot()

    def _set_textbox_value(self, value):
        self._ignore_textbox_callback = True
        try:
            self.textbox.set_val(f"{value}/{self.total}")
        finally:
            self._ignore_textbox_callback = False

    def _update_plot(self, initial=False):
        row = self.data_rows[self.idx]
        r = np.where(row == 0.0, np.nan, row)

        self.sc.set_offsets(np.column_stack([self.angles, r]))
        self.sc_highlight.set_offsets(
            np.column_stack(
                [self.angles[self.highlight_indices], r[self.highlight_indices]]
            )
        )

        self.ax.set_ylim(self.use_r_min, self.use_r_max)
        self._set_textbox_value(self.idx + 1)

        if initial:
            self.fig.canvas.draw()
        else:
            self.fig.canvas.draw_idle()


def main():
    data_rows = read_all_lines(FILENAME)
    if not data_rows:
        print("No data.")
        return

    angles = np.linspace(0, 2 * np.pi, NUM_POINTS, endpoint=False)

    LidarPlayer(
        data_rows=data_rows,
        angles=angles,
        use_r_min=r_min,
        use_r_max=r_max,
        pause_seconds=pause_seconds,
        save_frames=save_frames,
        out_dir=out_dir,
    )

    plt.show()


if __name__ == "__main__":
    main()
