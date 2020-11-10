import numpy as np

try:
    xmap = np.load("xmap.npy")
    ymap = np.load("ymap.npy")
except IOError:
    # MAPの作成
    COLS = 1280
    ROWS = 720
    xmap = np.zeros((ROWS, COLS), np.float32)
    ymap = np.zeros((ROWS, COLS), np.float32)
    DST_X = float(COLS)
    DST_Y = DST_X / 2
    SRC_CX1 = DST_X / 4
    SRC_CX2 = DST_X - SRC_CX1
    SRC_CY = DST_X / 4
    SRC_R = 0.884 * DST_X / 4
    SRC_RX = SRC_R * 1.00
    SRC_RY = SRC_R * 1.00
    #
    for y in range(COLS // 2):
        for x in range(COLS):
            ph1 = np.pi * x / DST_Y
            th1 = np.pi * y / DST_Y

            x1 = np.sin(th1) * np.cos(ph1)
            y1 = np.sin(th1) * np.sin(ph1)
            z1 = np.cos(th1)

            ph2 = np.arccos(-x1)
            th2 = (1 if y1 >= 0 else -1) * np.arccos(-z1 / np.sqrt(y1 * y1 + z1 * z1))

            if ph2 < np.pi / 2:
                r0 = ph2 / (np.pi / 2)
                xmap[y,x] = SRC_RX * r0 * np.cos(th2) + SRC_CX1
                ymap[y,x] = SRC_RY * r0 * np.sin(th2) + SRC_CY
            else:
                r0 = (np.pi - ph2) / (np.pi / 2)
                xmap[y,x] = SRC_RX * r0 * np.cos(np.pi - th2) + SRC_CX2
                ymap[y,x] = SRC_RY * r0 * np.sin(np.pi - th2) + SRC_CY

    np.save("xmap.npy", xmap)
    np.save("ymap.npy", ymap)
