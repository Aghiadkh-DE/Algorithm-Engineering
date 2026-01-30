#!/usr/bin/env python3
import csv
from collections import defaultdict

CSV_PATH = "benchmark_results.csv"
THREAD_PLOT = "speedup_vs_threads.svg"
SIZE_PLOT = "speedup_vs_size.svg"


def read_rows(path):
    rows = []
    with open(path, newline="") as f:
        reader = csv.DictReader(f)
        for row in reader:
            rows.append(
                {
                    "mode": row["mode"],
                    "size": int(row["size"]),
                    "threads": int(row["threads"]),
                    "algo": row["algo"],
                    "time_s": float(row["time_s"]),
                }
            )
    return rows


def save_svg_plot(file_path, series, title, x_label, y_label, x_ticks):
    width, height = 900, 520
    margin_left, margin_right = 80, 30
    margin_top, margin_bottom = 60, 70
    plot_w = width - margin_left - margin_right
    plot_h = height - margin_top - margin_bottom

    all_x = [x for points in series.values() for x, _ in points]
    all_y = [y for points in series.values() for _, y in points]

    min_x, max_x = min(all_x), max(all_x)
    min_y = 0.0
    max_y = max(all_y) if all_y else 1.0
    if max_y <= min_y:
        max_y = min_y + 1.0

    def x_to_px(x):
        if max_x == min_x:
            return margin_left + plot_w / 2
        return margin_left + (x - min_x) / (max_x - min_x) * plot_w

    def y_to_px(y):
        if max_y == min_y:
            return margin_top + plot_h / 2
        return margin_top + (1.0 - (y - min_y) / (max_y - min_y)) * plot_h

    colors = ["#1f77b4", "#d62728", "#2ca02c"]
    svg = []
    svg.append(f"<svg xmlns='http://www.w3.org/2000/svg' width='{width}' height='{height}'>")
    svg.append("<rect width='100%' height='100%' fill='white'/>")

    # Grid and axes
    svg.append(f"<line x1='{margin_left}' y1='{margin_top}' x2='{margin_left}' y2='{margin_top + plot_h}' stroke='#111' stroke-width='1.5'/>")
    svg.append(f"<line x1='{margin_left}' y1='{margin_top + plot_h}' x2='{margin_left + plot_w}' y2='{margin_top + plot_h}' stroke='#111' stroke-width='1.5'/>")

    # X ticks
    for x in x_ticks:
        px = x_to_px(x)
        svg.append(f"<line x1='{px}' y1='{margin_top + plot_h}' x2='{px}' y2='{margin_top + plot_h + 6}' stroke='#111'/>")
        svg.append(f"<text x='{px}' y='{margin_top + plot_h + 24}' font-size='12' text-anchor='middle'>{x}</text>")
        svg.append(f"<line x1='{px}' y1='{margin_top}' x2='{px}' y2='{margin_top + plot_h}' stroke='#ddd'/>")

    # Y ticks (5)
    for i in range(6):
        y = min_y + (max_y - min_y) * i / 5
        py = y_to_px(y)
        svg.append(f"<line x1='{margin_left - 6}' y1='{py}' x2='{margin_left}' y2='{py}' stroke='#111'/>")
        svg.append(f"<text x='{margin_left - 10}' y='{py + 4}' font-size='12' text-anchor='end'>{y:.2f}</text>")
        svg.append(f"<line x1='{margin_left}' y1='{py}' x2='{margin_left + plot_w}' y2='{py}' stroke='#eee'/>")

    # Title and labels
    svg.append(f"<text x='{width / 2}' y='30' font-size='16' text-anchor='middle'>{title}</text>")
    svg.append(f"<text x='{width / 2}' y='{height - 20}' font-size='14' text-anchor='middle'>{x_label}</text>")
    svg.append(f"<text x='20' y='{height / 2}' font-size='14' text-anchor='middle' transform='rotate(-90 20,{height / 2})'>{y_label}</text>")

    # Series
    for idx, (name, points) in enumerate(series.items()):
        color = colors[idx % len(colors)]
        points = sorted(points, key=lambda p: p[0])
        path_data = "M " + " L ".join(f"{x_to_px(x):.1f} {y_to_px(y):.1f}" for x, y in points)
        svg.append(f"<path d='{path_data}' fill='none' stroke='{color}' stroke-width='2'/>")
        for x, y in points:
            svg.append(f"<circle cx='{x_to_px(x):.1f}' cy='{y_to_px(y):.1f}' r='3.5' fill='{color}'/>")

    # Legend
    legend_x = margin_left + plot_w - 170
    legend_y = margin_top + 10
    svg.append(f"<rect x='{legend_x}' y='{legend_y}' width='160' height='{20 * len(series) + 10}' fill='white' stroke='#ccc'/>")
    for idx, name in enumerate(series.keys()):
        color = colors[idx % len(colors)]
        y = legend_y + 20 * idx + 18
        svg.append(f"<line x1='{legend_x + 10}' y1='{y - 6}' x2='{legend_x + 30}' y2='{y - 6}' stroke='{color}' stroke-width='2'/>")
        svg.append(f"<text x='{legend_x + 35}' y='{y - 2}' font-size='12'>{name}</text>")

    svg.append("</svg>")

    with open(file_path, "w", encoding="utf-8") as f:
        f.write("\n".join(svg))


def plot_thread_speedup(rows):
    std_time_by_thread = {}
    for row in rows:
        if row["mode"] == "thread" and row["algo"] == "std_sort":
            std_time_by_thread[row["threads"]] = row["time_s"]

    thread_rows = [r for r in rows if r["mode"] == "thread" and r["algo"] != "std_sort"]
    if not thread_rows:
        raise RuntimeError("No thread sweep data found.")

    series = defaultdict(list)
    for row in thread_rows:
        base = std_time_by_thread.get(row["threads"])
        if base is None:
            raise RuntimeError(f"Missing std::sort baseline for thread {row['threads']}")
        speedup = base / row["time_s"]
        if row["algo"] == "min_max":
            label = "min_max_quicksort"
        else:
            label = "__gnu_parallel::sort"
        series[label].append((row["threads"], speedup))

    x_ticks = sorted({row["threads"] for row in thread_rows})
    save_svg_plot(
        THREAD_PLOT,
        series,
        f"Speedup over std::sort vs Threads (n={thread_rows[0]['size']})",
        "Threads",
        "Relative speedup over std::sort",
        x_ticks,
    )


def plot_size_speedup(rows):
    std_time = {}
    for row in rows:
        if row["mode"] == "size" and row["algo"] == "std_sort":
            std_time[row["size"]] = row["time_s"]

    size_rows = [r for r in rows if r["mode"] == "size" and r["algo"] != "std_sort"]
    if not size_rows:
        raise RuntimeError("No size sweep data found.")

    series = defaultdict(list)
    for row in size_rows:
        base = std_time.get(row["size"])
        if base is None:
            raise RuntimeError(f"Missing std::sort baseline for size {row['size']}")
        speedup = base / row["time_s"]
        if row["algo"] == "min_max":
            label = "min_max_quicksort"
        else:
            label = "__gnu_parallel::sort"
        series[label].append((row["size"], speedup))

    x_ticks = sorted({row["size"] for row in size_rows})
    save_svg_plot(
        SIZE_PLOT,
        series,
        "Speedup over std::sort vs Array Size (all threads)",
        "Array size (n)",
        "Relative speedup over std::sort",
        x_ticks,
    )


if __name__ == "__main__":
    data = read_rows(CSV_PATH)
    plot_thread_speedup(data)
    plot_size_speedup(data)
    print(f"Wrote {THREAD_PLOT} and {SIZE_PLOT}")
