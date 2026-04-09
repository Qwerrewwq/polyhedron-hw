import numpy as np


class R3(np.ndarray):
    """ Вектор (точка) в R3 """

    def __new__(cls, x, y, z):
        obj = np.asarray([x, y, z]).view(cls)
        return obj

    @property
    def x(self):
        return self[0]

    @property
    def y(self):
        return self[1]

    @property
    def z(self):
        return self[2]

    # Поворот вокруг оси Oz
    def rz(self, fi):
        c, s = np.cos(fi), np.sin(fi)
        return R3(
            c * self.x - s * self.y,
            s * self.x + c * self.y,
            self.z
        )

    # Поворот вокруг оси Oy
    def ry(self, fi):
        c, s = np.cos(fi), np.sin(fi)
        return R3(
            c * self.x + s * self.z,
            self.y,
            -s * self.x + c * self.z
        )

    # Скалярное произведение
    def dot(self, other):
        return np.dot(self, other)

    # Векторное произведение
    def cross(self, other):
        return R3(*np.cross(self, other))


if __name__ == "__main__":
    x = R3(1.0, 1.0, 1.0)
    print("x", type(x), x)
    y = x + R3(1.0, -1.0, 0.0)
    print("y", type(y), y)
    y = y.rz(1.0)
    print("y", type(y), y)
    u = x.dot(y)
    print("u", type(u), u)
    v = x.cross(y)
    print("v", type(v), v)
