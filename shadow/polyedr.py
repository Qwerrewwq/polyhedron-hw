import numpy as np
from math import pi
from functools import reduce
from operator import add
from common.r3 import R3


def _get_xy_funcs():
    """Lazy load x, y functions from tk_drawer to avoid circular dependency"""
    from common.tk_drawer import x, y
    return x, y


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
        
        # Numpy массивы для оптимизированных вычислений
        self.vertexes_array = None

        # список строк файла
        with open(file) as f:
            lines = f.readlines()
            
        # Первая строка: коэффициент гомотетии и углы Эйлера
        buf = lines[0].split()
        c = float(buf.pop(0))
        alpha, beta, gamma = (float(x) * pi / 180.0 for x in buf)
        
        # Вторая строка: число вершин, граней и рёбер
        nv, nf, ne = (int(x) for x in lines[1].split())
        
        # Чтение всех вершин
        raw_vertices = []
        for i in range(2, nv + 2):
            x, y, z = (float(x) for x in lines[i].split())
            raw_vertices.append((x, y, z))
        
        # Массовое применение поворота и масштабирования с помощью numpy
        rotated = R3.rotate_vertices(raw_vertices, alpha, beta, gamma)
        self.vertexes_array = rotated * c
        
        # Создание объектов R3 для обратной совместимости
        for i in range(nv):
            self.vertexes.append(R3(*self.vertexes_array[i]))
        
        # Чтение граней и создание рёбер
        line_idx = nv + 2
        for _ in range(nf):
            buf = lines[line_idx].split()
            line_idx += 1
            size = int(buf.pop(0))
            vertex_indices = [int(n) - 1 for n in buf]
            vertexes = list(self.vertexes[n] for n in vertex_indices)
            
            # Задание рёбер грани
            for n in range(size):
                self.edges.append(Edge(vertexes[n - 1], vertexes[n]))
            
            # Задание самой грани
            self.facets.append(Facet(vertexes))

    # Метод изображения полиэдра
    def draw(self, tk):
        tk.clean()
        
        # Обработка теней для всех рёбер
        for e in self.edges:
            for f in self.facets:
                e.shadow(f)
        
        # Подготовка координат для массовой отрисовки
        lines_coords = []
        x_func, y_func = _get_xy_funcs()
        for e in self.edges:
            for s in e.gaps:
                p1 = e.r3(s.beg)
                p2 = e.r3(s.fin)
                coord1 = (x_func(p1), y_func(p1))
                coord2 = (x_func(p2), y_func(p2))
                lines_coords.append((coord1, coord2))
        
        # Массовая отрисовка всех линий
        tk.draw_lines_batch(lines_coords)
