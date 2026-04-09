import numpy as np
from math import pi, cos, sin
from functools import reduce
from operator import add
from concurrent.futures import ThreadPoolExecutor
from common.r3 import R3


class Segment:
    """ Одномерный отрезок """
    # Параметры конструктора: начало и конец отрезка (числа)

    def __init__(self, beg, fin):
        self.beg, self.fin = beg, fin

    # Отрезок вырожден?
    def is_degenerate(self):
        return self.beg >= self.fin

    # Пересечение с отрезком
    def intersect(self, other):
        if other.beg > self.beg:
            self.beg = other.beg
        if other.fin < self.fin:
            self.fin = other.fin
        return self

    # Разность отрезков
    # Разность двух отрезков всегда является списком из двух отрезков!
    def subtraction(self, other):
        return [Segment(
            self.beg, self.fin if self.fin < other.beg else other.beg),
            Segment(self.beg if self.beg > other.fin else other.fin, self.fin)]


class Edge:
    """ Ребро полиэдра """
    # Начало и конец стандартного одномерного отрезка
    SBEG, SFIN = 0.0, 1.0

    # Параметры конструктора: начало и конец ребра (точки в R3)
    def __init__(self, beg, fin):
        self.beg, self.fin = beg, fin
        # Список «просветов»
        self.gaps = [Segment(Edge.SBEG, Edge.SFIN)]

    # Учёт тени от одной грани
    def shadow(self, facet):
        # «Вертикальная» грань не затеняет ничего
        if facet.is_vertical():
            return
        # Нахождение одномерной тени на ребре
        shade = Segment(Edge.SBEG, Edge.SFIN)
        for u, v in zip(facet.vertexes, facet.v_normals()):
            shade.intersect(self.intersect_edge_with_normal(u, v))
            if shade.is_degenerate():
                return

        shade.intersect(
            self.intersect_edge_with_normal(
                facet.vertexes[0], facet.h_normal()))
        if shade.is_degenerate():
            return
        # Преобразование списка «просветов», если тень невырождена
        gaps = [s.subtraction(shade) for s in self.gaps]
        self.gaps = [
            s for s in reduce(add, gaps, []) if not s.is_degenerate()]

    # Преобразование одномерных координат в трёхмерные
    def r3(self, t):
        return self.beg * (Edge.SFIN - t) + self.fin * t

    # Пересечение ребра с полупространством, задаваемым точкой (a)
    # на плоскости и вектором внешней нормали (n) к ней
    def intersect_edge_with_normal(self, a, n):
        f0, f1 = n.dot(self.beg - a), n.dot(self.fin - a)
        if f0 >= 0.0 and f1 >= 0.0:
            return Segment(Edge.SFIN, Edge.SBEG)
        if f0 < 0.0 and f1 < 0.0:
            return Segment(Edge.SBEG, Edge.SFIN)
        x = - f0 / (f1 - f0)
        return Segment(Edge.SBEG, x) if f0 < 0.0 else Segment(x, Edge.SFIN)


class Facet:
    """ Грань полиэдра """
    # Параметры конструктора: список вершин

    def __init__(self, vertexes):
        self.vertexes = vertexes

    # «Вертикальна» ли грань?
    def is_vertical(self):
        return self.h_normal().dot(Polyedr.V) == 0.0

    # Нормаль к «горизонтальному» полупространству
    def h_normal(self):
        n = (
            self.vertexes[1] - self.vertexes[0]).cross(
            self.vertexes[2] - self.vertexes[0])
        return n * (-1.0) if n.dot(Polyedr.V) < 0.0 else n

    # Нормали к «вертикальным» полупространствам, причём k-я из них
    # является нормалью к грани, которая содержит ребро, соединяющее
    # вершины с индексами k-1 и k
    def v_normals(self):
        return [self._vert(x) for x in range(len(self.vertexes))]

    # Вспомогательный метод
    def _vert(self, k):
        n = (self.vertexes[k] - self.vertexes[k - 1]).cross(Polyedr.V)
        return n * \
            (-1.0) if n.dot(self.vertexes[k - 1] - self.center()) < 0.0 else n

    # Центр грани
    def center(self):
        return sum(self.vertexes, R3(0.0, 0.0, 0.0)) * \
            (1.0 / len(self.vertexes))


class Polyedr:
    """ Полиэдр """
    # вектор проектирования
    V = R3(0.0, 0.0, 1.0)

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

    # Метод изображения полиэдра с многопоточной обработкой теней
    def draw(self, tk):
        tk.clean()
        
        # Функция для обработки одного ребра
        def process_edge(e):
            for f in self.facets:
                e.shadow(f)
            return e
        
        # Используем ThreadPoolExecutor для параллельной обработки рёбер
        num_workers = min(len(self.edges), 8)  # Ограничиваем количество потоков
        with ThreadPoolExecutor(max_workers=num_workers) as executor:
            processed_edges = list(executor.map(process_edge, self.edges))
        
        # Рисуем все рёбра
        for e in processed_edges:
            for s in e.gaps:
                tk.draw_line(e.r3(s.beg), e.r3(s.fin))
