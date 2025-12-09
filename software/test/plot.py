# plot_lidar_scatter_custom_scale.py
import numpy as np
import matplotlib.pyplot as plt
from matplotlib.widgets import Button, TextBox
import sys
from pathlib import Path

# ---------------Configure area --------------
FILENAME =  "DATA.TXT" #"SQ.TXT" #"square.txt"
NUM_POINTS = 150
pause_seconds = 0.1          # interval time(s)
r_min = 0
r_max = 500
POINT_SIZE = 5               # data point size

save_frames = False         # 若 True，保存每一帧
out_dir = "frames"
# --------------------------------------------

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
            parts = [s.strip() for s in line.split(',') if s.strip() != ""]
            vals = parts[:NUM_POINTS]
            if len(vals) < NUM_POINTS:
                vals += ["0"] * (NUM_POINTS - len(vals))

            try:
                arr = np.array([float(v) for v in vals], dtype=float)
            except ValueError:
                print(f"Warning: line {lineno} has non-numeric, fill with zeros.", file=sys.stderr)
                arr = np.zeros(NUM_POINTS)

            rows.append(arr)
    return rows

class LidarPlayer:
    def __init__(self, data_rows, angles, use_r_min, use_r_max, pause_seconds, save_frames=False, out_dir="frames"):
        self.data_rows = data_rows
        self.angles = angles
        self.use_r_min = use_r_min
        self.use_r_max = use_r_max
        self.pause_seconds = pause_seconds
        self.save_frames = save_frames
        self.out_dir = out_dir

        self.total = len(data_rows)
        self.idx = 0
        self.paused = False

        # flag to avoid textbox callback recursion when programmatically updating its value
        self._ignore_textbox_callback = False

        # Setup figure & polar axis
        self.fig, self.ax = plt.subplots(figsize=(6,6), subplot_kw=dict(projection='polar'))
        # leave room for buttons at bottom and textbox at top
        plt.subplots_adjust(bottom=0.18, top=0.88)
        self.ax.set_ylim(self.use_r_min, self.use_r_max)

        # initial scatter
        r0 = np.where(data_rows[0] == 0.0, np.nan, data_rows[0])
        self.sc = self.ax.scatter(self.angles, r0, s=POINT_SIZE, alpha=0.75)

        # TextBox for showing/entering frame number (top center).
        # Now initial value displays "current/total", e.g. "1/240"
        ax_text = plt.axes([0.25, 0.90, 0.50, 0.05])   # x, y, w, h (normalized)
        self.textbox = TextBox(ax_text, 'Frame (cur/total):', initial=f"{self.idx + 1}/{self.total}")
        self.textbox.on_submit(self._on_text_submit)

        # Buttons: create axes for them (bottom)
        ax_pause = plt.axes([0.35, 0.05, 0.12, 0.06])  # x, y, w, h
        ax_prev  = plt.axes([0.12, 0.05, 0.12, 0.06])
        ax_next  = plt.axes([0.58, 0.05, 0.12, 0.06])

        self.btn_pause = Button(ax_pause, 'Pause')
        self.btn_prev  = Button(ax_prev, 'Prev')
        self.btn_next  = Button(ax_next, 'Next')

        self.btn_pause.on_clicked(self._on_pause_clicked)
        self.btn_prev.on_clicked(self._on_prev_clicked)
        self.btn_next.on_clicked(self._on_next_clicked)

        # matplotlib timer to drive frames (non-blocking)
        interval_ms = max(10, int(self.pause_seconds * 1000))
        self.timer = self.fig.canvas.new_timer(interval=interval_ms)
        self.timer.add_callback(self._on_timer)
        self.timer.start()

        # draw initial state
        self._update_plot(initial=True)

    def _on_pause_clicked(self, event):
        # toggle pause/play
        self.paused = not self.paused
        self.btn_pause.label.set_text('Play' if self.paused else 'Pause')

    def _on_prev_clicked(self, event):
        # go to previous frame and pause
        self.paused = True
        self.btn_pause.label.set_text('Play')
        self.idx = (self.idx - 1) % self.total
        self._update_plot()

    def _on_next_clicked(self, event):
        # go to next frame and pause
        self.paused = True
        self.btn_pause.label.set_text('Play')
        self.idx = (self.idx + 1) % self.total
        self._update_plot()

    def _on_text_submit(self, text):
        # Called when user presses Enter in the textbox.
        # Expect inputs like "123", "123/240", or other strings containing a number.
        if self._ignore_textbox_callback:
            return
        txt = text.strip()
        if not txt:
            # restore current display
            self._set_textbox_value(self.idx + 1)
            return
        # accept inputs like "123" or "123/240" (we'll parse first integer)
        import re
        m = re.search(r'(\d+)', txt)
        if not m:
            # invalid input, restore textbox to current frame
            self._set_textbox_value(self.idx + 1)
            return
        val = int(m.group(1))
        # clamp to range 1..total
        if val < 1:
            val = 1
        if val > self.total:
            val = self.total
        # go to that frame (convert to zero-based)
        self.idx = val - 1
        # pause after jumping
        self.paused = True
        self.btn_pause.label.set_text('Play')
        self._update_plot()

    def _on_timer(self):
        # called repeatedly by timer
        if not self.paused:
            self.idx = (self.idx + 1) % self.total
            self._update_plot()
        # timer keeps运行 automatically

    def _set_textbox_value(self, value):
        # update textbox without triggering its submit callback
        # value is current one-based frame number
        self._ignore_textbox_callback = True
        try:
            self.textbox.set_val(f"{value}/{self.total}")
        finally:
            self._ignore_textbox_callback = False

    def _update_plot(self, initial=False):
        row = self.data_rows[self.idx]
        r = np.where(row == 0.0, np.nan, row)
        offsets = np.column_stack([self.angles, r])
        # update scatter offsets
        self.sc.set_offsets(offsets)
        # update limits
        self.ax.set_ylim(self.use_r_min, self.use_r_max)
        # update textbox to display current frame / total
        self._set_textbox_value(self.idx + 1)
        # redraw
        if initial:
            self.fig.canvas.draw()
        else:
            self.fig.canvas.draw_idle()

        if self.save_frames:
            Path(self.out_dir).mkdir(exist_ok=True)
            self.fig.savefig(Path(self.out_dir) / f"frame_{self.idx:03d}.png")

def main():
    data_rows = read_all_lines(FILENAME)
    if not data_rows:
        print("No data.")
        return

    angles = np.linspace(0, 2*np.pi, NUM_POINTS, endpoint=False)

    use_r_min = r_min
    use_r_max = r_max

    player = LidarPlayer(
        data_rows=data_rows,
        angles=angles,
        use_r_min=use_r_min,
        use_r_max=use_r_max,
        pause_seconds=pause_seconds,
        save_frames=save_frames,
        out_dir=out_dir
    )

    # Show interactive plot (this call blocks until closed)
    plt.show()

if __name__ == "__main__":
    main()
