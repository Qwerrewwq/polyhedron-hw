import numpy as np
from math import sin, cos


class R3:
    """ Вектор (точка) в R3 """

    # Конструктор
    def __init__(self, x, y, z):
        self.data = np.array([x, y, z], dtype=np.float64)

    @property
    def x(self):
        return self.data[0]

    @property
    def y(self):
        return self.data[1]

    @property
    def z(self):
        return self.data[2]

    # Сумма векторов
    def __add__(self, other):
        return R3(*((self.data + other.data)))

    # Разность векторов
    def __sub__(self, other):
        return R3(*((self.data - other.data)))

    # Умножение на число
    def __mul__(self, k):
        return R3(*((self.data * k)))

    def __rmul__(self, k):
        return self.__mul__(k)

    # Поворот вокруг оси Oz
    def rz(self, fi):
        c, s = cos(fi), sin(fi)
        rot_matrix = np.array([
            [c, -s, 0],
            [s, c, 0],
            [0, 0, 1]
        ], dtype=np.float64)
        return R3(*np.dot(rot_matrix, self.data))

    # Поворот вокруг оси Oy
    def ry(self, fi):
        c, s = cos(fi), sin(fi)
        rot_matrix = np.array([
            [c, 0, s],
            [0, 1, 0],
            [-s, 0, c]
        ], dtype=np.float64)
        return R3(*np.dot(rot_matrix, self.data))

    # Скалярное произведение
    def dot(self, other):
        return float(np.dot(self.data, other.data))

    # Векторное произведение
    def cross(self, other):
        return R3(*np.cross(self.data, other.data))

    # Преобразование в numpy массив для совместимости
    def to_array(self):
        return self.data

    # Статический метод для массового преобразования координат
    @staticmethod
    def rotate_vertices(vertices, alpha, beta, gamma):
        """
        vertices: список кортежей (x, y, z)
        Возвращает numpy массив повернутых вершин shape (N, 3)
        """
        ca, sa = cos(alpha), sin(alpha)
        cb, sb = cos(beta), sin(beta)
        cg, sg = cos(gamma), sin(gamma)

        # Матрица поворота Rz(gamma) * Ry(beta) * Rz(alpha)
        Rz_a = np.array([
            [ca, -sa, 0],
            [sa, ca, 0],
            [0, 0, 1]
        ], dtype=np.float64)

        Ry_b = np.array([
            [cb, 0, sb],
            [0, 1, 0],
            [-sb, 0, cb]
        ], dtype=np.float64)

        Rz_g = np.array([
            [cg, -sg, 0],
            [sg, cg, 0],
            [0, 0, 1]
        ], dtype=np.float64)

        # Комбинированная матрица поворота
        R = Rz_g @ Ry_b @ Rz_a

        # Преобразуем входные данные в numpy массив
        v_array = np.asarray(vertices, dtype=np.float64)

        # Применяем поворот ко всем вершинам сразу
        return v_array @ R.T


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
