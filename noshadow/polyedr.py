import numpy as np
from math import pi, cos, sin
from common.r3 import R3


class Edge:
    """ Ребро полиэдра """
    # Параметры конструктора: начало и конец ребра (точки в R3)

    def __init__(self, beg, fin):
        self.beg, self.fin = beg, fin


class Facet:
    """ Грань полиэдра """
    # Параметры конструктора: список вершин

    def __init__(self, vertexes):
        self.vertexes = vertexes


class Polyedr:
    """ Полиэдр """
    # Параметры конструктора: файл, задающий полиэдр

    def __init__(self, file):

        # списки вершин, рёбер и граней полиэдра
        self.vertexes, self.edges, self.facets = [], [], []

        # список строк файла
        with open(file) as f:
            lines = f.readlines()

        # обрабатываем первую строку; buf - вспомогательный массив
        buf = lines[0].split()
        # коэффициент гомотетии
        c = float(buf.pop(0))
        # углы Эйлера, определяющие вращение
        alpha, beta, gamma = (float(x) * pi / 180.0 for x in buf)

        # во второй строке число вершин, граней и рёбер полиэдра
        nv, nf, ne = (int(x) for x in lines[1].split())

        # предзагрузка всех вершин для быстрого доступа
        raw_vertexes = []
        for i in range(2, nv + 2):
            x, y, z = (float(x) for x in lines[i].split())
            raw_vertexes.append((x, y, z))

        # Векторизованное применение трансформаций ко всем вершинам
        rot_z_alpha = np.array([
            [cos(alpha), -sin(alpha), 0],
            [sin(alpha), cos(alpha), 0],
            [0, 0, 1]], dtype=np.float64)
        rot_y_beta = np.array([
            [cos(beta), 0, sin(beta)],
            [0, 1, 0],
            [-sin(beta), 0, cos(beta)]], dtype=np.float64)
        rot_z_gamma = np.array([
            [cos(gamma), -sin(gamma), 0],
            [sin(gamma), cos(gamma), 0],
            [0, 0, 1]], dtype=np.float64)
        combined_rot = rot_z_gamma @ rot_y_beta @ rot_z_alpha

        # Применяем трансформации ко всем вершинам сразу через numpy
        raw_arr = np.array(raw_vertexes, dtype=np.float64)
        transformed = (raw_arr @ combined_rot.T) * c

        # Создаём объекты R3 из трансформированных данных
        for i in range(nv):
            self.vertexes.append(R3(transformed[i, 0], transformed[i, 1], transformed[i, 2]))

        # Обработка граней и рёбер
        line_idx = nv + 2
        while line_idx < len(lines):
            buf = lines[line_idx].split()
            line_idx += 1
            # количество вершин очередной грани
            size = int(buf.pop(0))
            # массив вершин этой грани
            vertexes = [self.vertexes[int(n) - 1] for n in buf]
            # задание рёбер грани
            for n in range(size):
                self.edges.append(Edge(vertexes[n - 1], vertexes[n]))
            # задание самой грани
            self.facets.append(Facet(vertexes))

    # Метод изображения полиэдра
    def draw(self, tk):
        tk.clean()
        for e in self.edges:
            tk.draw_line(e.beg, e.fin)
        tk.flush()  # Обновляем экран один раз после отрисовки всех линий
