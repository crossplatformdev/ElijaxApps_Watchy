from PIL import Image
import os.path
import math
import sys
import importlib 

spec = importlib.util.spec_from_file_location("watchy-image-editor", f'{os.path.dirname(__file__)}/../../watchy-image-editor/__init__.py')
WatchyImageEditorModule = importlib.util.module_from_spec(spec)
sys.modules["watchy-image-editor"] = WatchyImageEditorModule
spec = importlib.util.spec_from_file_location("watchy-image-editor.image", f'{os.path.dirname(__file__)}/../../watchy-image-editor/image.py')
BmpImageModule = importlib.util.module_from_spec(spec)
sys.modules["watchy-image-editor.image"] = BmpImageModule
spec.loader.exec_module(BmpImageModule)

BmpImage = getattr(BmpImageModule, "Image", None)

UI_IMAGES = [("pokemon_1", 96, 16) , ("pokemon_2", 96, 26), ("pokemon_3", 200, 58)]
UI_OTHER_IMAGES = [("bar_0", 8, 4), ("bar_1", 8, 4), ("cursor", 8, 9)]

DIRS = ["red-blue", "red-green", "yellow"]

CWD = os.path.dirname(__file__)
ROOT_DIR = f"{CWD}/pokemon-src/main-sprites"

COUNT = 151

OUT_DIR = f"{CWD}/pokemon-out"

WIDTH = 72
HEIGHT = 72

D_DATA = [
    [
        [0]
    ],
    [
        [1, 0, 0, 0],
        [0, 0, 0, 0],
        [0, 0, 1, 0],
        [0, 0, 0, 0]
    ],
    [
        [1, 0],
        [0, 0]
    ],
    [
        [1, 0, 1, 0],
        [0, 1, 0, 0],
        [1, 0, 1, 0],
        [0, 0, 0, 1]
    ],
    [
        [1, 0],
        [0, 1]
    ]
]

D_LEVELS = len(D_DATA) * 2 - 1

def make_dithering(i: int) -> Image.Image:
    im = Image.new("L", (WIDTH, WIDTH), 'white')
    if i < len(D_DATA):
        d = D_DATA[i]
    else:
        d = D_DATA[D_LEVELS - i - 1]
    w = len(d[0])
    h = len(d)
    for x in range(WIDTH):
        for y in range(WIDTH):
            v = d[y % h][x % w]
            if i < len(D_DATA):
                im.putpixel((x, y), 256 if v else 0)
            else:
                im.putpixel((x, y), 0 if v else 256)
    return im

D_IM = [make_dithering(i) for i in range(D_LEVELS)]

def make_extent() -> Image.Image:
    im = Image.new("L", (WIDTH - HEIGHT, HEIGHT), color=0)
    for x in range(WIDTH - HEIGHT):
        for y in range(HEIGHT):
            im.putpixel((x, y), 256)
    return im

EXTENT = make_extent()

def make_mask(im: Image.Image, i: int) -> Image.Image:
    return im.point(lambda p: 1 if p >= math.floor(i * 256 / D_LEVELS) and p < math.ceil((i + 1) * 256 / D_LEVELS) else 0, mode="1")

def convert(im: Image.Image) -> Image.Image:
    im = im.resize((WIDTH, HEIGHT))
    im0, im = im, Image.new(im.mode, (WIDTH, HEIGHT), color=1)
    for i in range(D_LEVELS):
        im.paste(D_IM[i], (0, 0), mask=make_mask(im0, i))
        
    im.paste(EXTENT, (2 * WIDTH - HEIGHT, 0))
    im = im.convert(mode="P")
    return im

def convert_front(im: Image.Image) -> Image.Image:
    w0, h0 = im_src.size
    im = Image.new(im_src.mode, (56, 56), 'white')
    im.paste(im_src, ((56 - w0) // 2, (56 - h0) // 2))
    return convert(im)

def convert_back(im_src: Image.Image) -> Image.Image:
    im = Image.new(im_src.mode, (28, 28), 'white')
    im.paste(im_src, (0, 0))
    return convert(im)

if __name__ == '__main__':
    for lang in ["en", "fr"]:
        with open(f"{CWD}/ui_{lang}.h", mode='w') as f:
            for name, width, height in UI_IMAGES:
                im_bmp = BmpImage(name, width, height)
                im_bmp.import_bmp(f"{lang}/{name}.bmp")
                f.write(im_bmp.export_cpp()+"\n")

    with open(f"{CWD}/ui_other.h", mode='w') as f:
        for name, width, height in UI_OTHER_IMAGES:
            im_bmp = BmpImage(name, width, height)
            im_bmp.import_bmp(f"other/{name}.bmp")
            f.write(im_bmp.export_cpp()+"\n")


    try:
        os.mkdir(f"{OUT_DIR}")
    except:
        pass
    for sub in DIRS:
        try:
            os.mkdir(f"{OUT_DIR}/{sub}")
        except:
            pass
        with open(f"{CWD}/pokemon_{sub.replace('-', '_')}.h", mode='w') as f:
            for i in range(1, COUNT + 1):
                try:
                    with Image.open(f"{ROOT_DIR}/{sub}/gray/{i}.png") as im_src:
                        im_out = convert_front(im_src)
                        im_out.save(f"{OUT_DIR}/{sub}/front_{i}.bmp")
                except:
                    pass
                im_bmp = BmpImage(f"front_{i}", WIDTH, HEIGHT)
                im_bmp.import_bmp(f"{OUT_DIR}/{sub}/front_{i}.bmp")
                f.write(im_bmp.export_cpp()+"\n")
                    
                try:
                    with Image.open(f"{ROOT_DIR}/{sub}/back/gray/{i}.png") as im_src:
                        im_out = convert_back(im_src)
                        im_out.save(f"{OUT_DIR}/{sub}/back_{i}.bmp")
                except:
                    pass
                im_bmp = BmpImage(f"back_{i}", WIDTH, HEIGHT)
                im_bmp.import_bmp(f"{OUT_DIR}/back_{i}.bmp")
                f.write(im_bmp.export_cpp()+"\n")