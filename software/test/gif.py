# plot_lidar_scatter_custom_scale.py
import numpy as np
import matplotlib.pyplot as plt
from matplotlib.widgets import Button, TextBox
import sys
from pathlib import Path
import imageio 

# ---------------Configure area --------------
FILENAME = "data.txt"#DATA.TXT" #"SQ.TXT" #"square.txt"
NUM_POINTS = 200
pause_seconds = 0.1             # interval time(s)
r_min = 0
r_max = 2000
POINT_SIZE = 5                  # data point size

save_frames = True            # <<-- 确保设置为 True
out_dir = "frames"
output_gif_filename = "lidar_animation.gif" # <<-- 定义输出 GIF 文件名
# --------------------------------------------

def read_all_lines(fname):
    # ... (read_all_lines 函数保持不变)
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

# ... (LidarPlayer 类定义保持不变)
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
        ax_text = plt.axes([0.25, 0.90, 0.50, 0.05])  # x, y, w, h (normalized)
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
            # 使用 f-string 格式化文件名，确保顺序正确
            self.fig.savefig(Path(self.out_dir) / f"frame_{self.idx:03d}.png")


# <<-- 新增函数：将图片合并为 GIF
def create_gif_from_frames(frame_dir, output_filename, duration):
    """
    读取指定目录下的所有 PNG 文件并将其合并为 GIF 动画。
    
    :param frame_dir: 包含 PNG 帧图片的目录。
    :param output_filename: 输出 GIF 文件的路径。
    :param duration: 每帧持续时间（以秒为单位）。
    """
    print(f"\nMerging frames from '{frame_dir}' into '{output_filename}'...")
    
    # 查找所有 PNG 文件，并按文件名排序，确保帧的顺序正确
    frame_files = sorted(Path(frame_dir).glob("frame_*.png"))
    
    if not frame_files:
        print("Warning: No frames found to create GIF.")
        return

    # 读取所有帧图片
    images = []
    for file in frame_files:
        # 使用 imageio 读取图片
        images.append(imageio.v3.imread(file)) 

    # 将图片写入 GIF 文件
    # duration = 0.1 表示每帧显示 0.1 秒
    imageio.v3.imwrite(
        output_filename, 
        images, 
        duration=duration, # duration 是每帧的持续时间（秒）
        loop=0             # loop=0 表示无限循环
    )
    print(f"GIF created successfully at '{output_filename}'")
    
    # 可选：删除临时帧文件
    # for file in frame_files:
    #     file.unlink()
    # Path(frame_dir).rmdir()
    # print(f"Temporary frames in '{frame_dir}' deleted.")

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

    # <<-- 在 plt.show() 关闭后执行 GIF 合并
    if save_frames:
        # duration 参数使用 pause_seconds，因为这是您设定的帧间隔时间
        # 如果 pause_seconds 设置太小，GIF 可能会播放得很快。
        gif_duration = pause_seconds 
        create_gif_from_frames(out_dir, output_gif_filename, gif_duration)


if __name__ == "__main__":
    main()