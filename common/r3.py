import numpy as np
from math import sin, cos


class R3:
    """ Вектор (точка) в R3 """

    # Конструктор
    def __init__(self, x, y, z):
        self._data = np.array([x, y, z], dtype=np.float64)

    @property
    def x(self):
        return self._data[0]

    @property
    def y(self):
        return self._data[1]

    @property
    def z(self):
        return self._data[2]

    # Сумма векторов
    def __add__(self, other):
        return R3(*(self._data + other._data))

    # Разность векторов
    def __sub__(self, other):
        return R3(*(self._data - other._data))

    # Умножение на число
    def __mul__(self, k):
        return R3(*(self._data * k))

    # Поворот вокруг оси Oz
    def rz(self, fi):
        c, s = cos(fi), sin(fi)
        rot = np.array([[c, -s, 0], [s, c, 0], [0, 0, 1]], dtype=np.float64)
        return R3(*np.dot(rot, self._data))

    # Поворот вокруг оси Oy
    def ry(self, fi):
        c, s = cos(fi), sin(fi)
        rot = np.array([[c, 0, s], [0, 1, 0], [-s, 0, c]], dtype=np.float64)
        return R3(*np.dot(rot, self._data))

    # Скалярное произведение
    def dot(self, other):
        return np.dot(self._data, other._data)

    # Векторное произведение
    def cross(self, other):
        return R3(*np.cross(self._data, other._data))


if __name__ == "__main__":
    x = R3(1.0, 1.0, 1.0)
    print("x", type(x), x.__dict__)
    y = x + R3(1.0, -1.0, 0.0)
    print("y", type(y), y.__dict__)
    y = y.rz(1.0)
    print("y", type(y), y.__dict__)
    u = x.dot(y)
    print("u", type(u), u)
    v = x.cross(y)
    print("v", type(v), v.__dict__)
