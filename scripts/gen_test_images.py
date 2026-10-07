# -*- coding: utf-8 -*-
"""生成留素图像层测试图集（一次性脚本，产物提交到 tests/fixtures/）。

图集内容：
- exif1.jpg ~ exif8.jpg : 带 EXIF 方向标记 1~8 的 JPEG，内容为四象限色块
  （左上红 R、右上蓝 B、左下绿 G、右下黄 Y），用于验证解码端自动方向；
- plain.png : 无 EXIF 的四象限 PNG（方向基准）；
- big.png   : 1200×900 四象限 PNG（缩略图缩放测试）；
- corrupt.jpg : 截断的 JPEG（解码失败样本）；
- notimage.txt  : 非图片文件。

运行：py scripts/gen_test_images.py
依赖：pip install pillow
"""
import os

from PIL import Image

HERE = os.path.dirname(os.path.abspath(__file__))
OUT = os.path.normpath(os.path.join(HERE, "..", "tests", "fixtures"))

W, H = 40, 30
# 四象限颜色（饱和度足够高，允许 JPEG 压缩误差 ±30）
RED = (220, 40, 40)
BLUE = (40, 60, 220)
GREEN = (40, 170, 60)
YELLOW = (235, 210, 40)


def quadrant_image():
    img = Image.new("RGB", (W, H), (255, 255, 255))
    px = img.load()
    for x in range(W):
        for y in range(H):
            if x < W // 2 and y < H // 2:
                px[x, y] = RED       # 左上
            elif x >= W // 2 and y < H // 2:
                px[x, y] = BLUE      # 右上
            elif x < W // 2 and y >= H // 2:
                px[x, y] = GREEN     # 左下
            else:
                px[x, y] = YELLOW    # 右下
    return img


def main():
    os.makedirs(OUT, exist_ok=True)
    base = quadrant_image()

    for orientation in range(1, 9):
        ex = Image.Exif()
        ex[274] = orientation  # 0x0112 Orientation
        path = os.path.join(OUT, f"exif{orientation}.jpg")
        base.save(path, "JPEG", quality=95, exif=ex)
        print("写入", path)

    plain_path = os.path.join(OUT, "plain.png")
    base.save(plain_path, "PNG")
    print("写入", plain_path)

    big = quadrant_image().resize((1200, 900), Image.NEAREST)
    big_path = os.path.join(OUT, "big.png")
    big.save(big_path, "PNG")
    print("写入", big_path)

    # 截断 JPEG：取 exif1.jpg 前 300 字节
    with open(os.path.join(OUT, "exif1.jpg"), "rb") as f:
        head = f.read(300)
    corrupt = os.path.join(OUT, "corrupt.jpg")
    with open(corrupt, "wb") as f:
        f.write(head)
    print("写入", corrupt)

    notimage = os.path.join(OUT, "notimage.txt")
    with open(notimage, "w", encoding="utf-8") as f:
        f.write("这不是一张图片。")
    print("写入", notimage)


if __name__ == "__main__":
    main()
