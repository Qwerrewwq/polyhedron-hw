from pytest import approx
from common.r3 import R3
from shadow.polyedr import Segment, Edge, Facet, Polyedr


def r3approx(self, other):
    return self.x == approx(other.x) and self.y == approx(other.y) and \
        self.z == approx(other.z)


setattr(R3, 'approx', r3approx)


def seg_approx(self, other):
    return self.beg == approx(other.beg) and self.fin == approx(other.fin) or \
        self.beg == approx(other.fin) and self.fin == approx(other.beg)


setattr(Segment, 'approx', seg_approx)


class TestGoodPoints:
    """Тесты для проверки определения «хороших» точек"""

    def test_good_point_inside_annulus(self):
        """Точка с проекцией в кольце 1 < r^2 < 4 — хорошая"""
        # Точка (1.2, 0, 0): r^2 = 1.44, что между 1 и 4
        p = R3(1.2, 0.0, 0.0)
        assert Edge.is_good_point(p) is True

    def test_good_point_inside_annulus_2(self):
        """Точка с проекцией в кольце 1 < r^2 < 4 — хорошая"""
        # Точка (1.5, 1.0, 0): r^2 = 2.25 + 1 = 3.25, что между 1 и 4
        p = R3(1.5, 1.0, 0.0)
        assert Edge.is_good_point(p) is True

    def test_bad_point_inside_inner_circle(self):
        """Точка с проекцией внутри x^2+y^2=1 — плохая"""
        # Точка (0.5, 0, 0): r^2 = 0.25 < 1
        p = R3(0.5, 0.0, 0.0)
        assert Edge.is_good_point(p) is False

    def test_bad_point_on_inner_circle(self):
        """Точка на границе x^2+y^2=1 — плохая (строго вне)"""
        # Точка (1, 0, 0): r^2 = 1
        p = R3(1.0, 0.0, 0.0)
        assert Edge.is_good_point(p) is False

    def test_bad_point_on_outer_circle(self):
        """Точка на границе x^2+y^2=4 — плохая (строго внутри)"""
        # Точка (2, 0, 0): r^2 = 4
        p = R3(2.0, 0.0, 0.0)
        assert Edge.is_good_point(p) is False

    def test_bad_point_outside_outer_circle(self):
        """Точка с проекцией вне x^2+y^2=4 — плохая"""
        # Точка (3, 0, 0): r^2 = 9 > 4
        p = R3(3.0, 0.0, 0.0)
        assert Edge.is_good_point(p) is False

    def test_good_point_negative_coords(self):
        """Точка с отрицательными координатами, но в кольце — хорошая"""
        # Точка (-1.2, -1.0, 0): r^2 = 1.44 + 1 = 2.44, что между 1 и 4
        p = R3(-1.2, -1.0, 0.0)
        assert Edge.is_good_point(p) is True

    def test_edge_length_calculation(self):
        """Проверка вычисления длины ребра"""
        p1 = R3(0.0, 0.0, 0.0)
        p2 = R3(1.0, 0.0, 0.0)
        assert Edge.edge_length(p1, p2) == approx(1.0)

    def test_edge_length_calculation_3d(self):
        """Проверка вычисления длины ребра в 3D"""
        p1 = R3(0.0, 0.0, 0.0)
        p2 = R3(1.0, 1.0, 1.0)
        expected = (1.0**2 + 1.0**2 + 1.0**2) ** 0.5
        assert Edge.edge_length(p1, p2) == approx(expected)


class TestGoodEdgesWithSimplePolyedrs:
    """Тесты с простыми специально сконструированными полиэдрами"""

    def test_simple_polyedr_all_good_vertices(self):
        """
        Простой тетраэдр, все вершины которого имеют проекции в кольце.
        Файл: data/test_all_good.geom
        Вершины после трансформации должны иметь 1 < x^2+y^2 < 4
        """
        # Создаём простой тестовый полиэдр программно
        # Используем коэффициент гомотетии c=1 и углы 0, чтобы контролировать координаты
        # Вершины: (1.2, 0, 0), (0, 1.2, 0), (-1.2, 0, 0), (0, -1.2, 0.5)
        # Все имеют r^2 = 1.44, что между 1 и 4
        
        # Создадим временный файл с геометрией
        import tempfile
        import os
        
        geom_content = """1.0 0.0 0.0 0.0
4 4 6
1.2 0.0 0.0
0.0 1.2 0.0
-1.2 0.0 0.0
0.0 -1.2 0.5
3 1 2 3
3 1 3 4
3 1 4 2
3 2 4 3
"""
        with tempfile.NamedTemporaryFile(mode='w', suffix='.geom', delete=False) as f:
            f.write(geom_content)
            temp_file = f.name
        
        try:
            p = Polyedr(temp_file)
            # Все 4 вершины имеют r^2 = 1.44, значит все 6 рёбер должны учитываться
            result = p.good_edges_length_sum()
            
            # Вычислим ожидаемую сумму длин всех рёбер
            vertices = p.vertexes
            edges = p.edges
            
            # Проверим, что все вершины действительно хорошие
            for v in vertices:
                assert Edge.is_good_point(v), f"Вершина {v} не является хорошей"
            
            # Все рёбра должны быть учтены
            expected_sum = sum(Edge.edge_length(e.beg, e.fin) for e in edges)
            assert result == approx(expected_sum)
        finally:
            os.unlink(temp_file)

    def test_simple_polyedr_no_good_vertices(self):
        """
        Простой полиэдр, ни одна вершина которого не является хорошей.
        Все вершины имеют r^2 <= 1 (внутри внутренней окружности)
        """
        import tempfile
        import os
        
        geom_content = """1.0 0.0 0.0 0.0
4 4 6
0.5 0.0 0.0
0.0 0.5 0.0
-0.5 0.0 0.0
0.0 -0.5 0.5
3 1 2 3
3 1 3 4
3 1 4 2
3 2 4 3
"""
        with tempfile.NamedTemporaryFile(mode='w', suffix='.geom', delete=False) as f:
            f.write(geom_content)
            temp_file = f.name
        
        try:
            p = Polyedr(temp_file)
            result = p.good_edges_length_sum()
            
            # Все вершины имеют r^2 = 0.25 < 1, значит сумма должна быть 0
            assert result == approx(0.0)
        finally:
            os.unlink(temp_file)

    def test_simple_polyedr_mixed_vertices(self):
        """
        Простой полиэдр с混合 вершинами: некоторые хорошие, некоторые нет.
        """
        import tempfile
        import os
        
        # Вершина 1: (1.5, 0, 0) -> r^2 = 2.25 (хорошая)
        # Вершина 2: (0.5, 0, 0) -> r^2 = 0.25 (плохая)
        # Вершина 3: (0, 1.5, 0) -> r^2 = 2.25 (хорошая)
        # Вершина 4: (0, 0.5, 0.5) -> r^2 = 0.25 (плохая)
        # Рёбра: 1-2, 2-3, 3-1 (грань 1), 1-4, 2-4, 3-4 (остальные грани)
        # Хорошие рёбра: только те, где обе вершины хорошие
        # В данном случае: нет рёбер с двумя хорошими вершинами (1-3 не соединены напрямую в первой грани)
        # На самом деле в грани 1: 1-2, 2-3, 3-1
        # Ребро 3-1: обе вершины хорошие!
        
        geom_content = """1.0 0.0 0.0 0.0
4 4 6
1.5 0.0 0.0
0.5 0.0 0.0
0.0 1.5 0.0
0.0 0.5 0.5
3 1 2 3
3 1 3 4
3 1 4 2
3 2 4 3
"""
        with tempfile.NamedTemporaryFile(mode='w', suffix='.geom', delete=False) as f:
            f.write(geom_content)
            temp_file = f.name
        
        try:
            p = Polyedr(temp_file)
            result = p.good_edges_length_sum()
            
            # Найдём все рёбра с двумя хорошими вершинами
            good_edges = [e for e in p.edges 
                         if Edge.is_good_point(e.beg) and Edge.is_good_point(e.fin)]
            
            expected_sum = sum(Edge.edge_length(e.beg, e.fin) for e in good_edges)
            assert result == approx(expected_sum)
        finally:
            os.unlink(temp_file)

    def test_cube_with_scaling(self):
        """
        Тест с кубом и коэффициентом гомотетии, который помещает некоторые вершины в кольцо.
        """
        # Куб из cube.geom имеет вершины с координатами +/-0.5
        # После умножения на c=200 и поворота, проверим результат
        p = Polyedr("data/cube.geom")
        
        # Просто проверяем, что метод работает и возвращает неотрицательное число
        result = p.good_edges_length_sum()
        assert result >= 0.0
        
        # Проверим, что все вершины корректно классифицируются
        for v in p.vertexes:
            r_squared = v.x * v.x + v.y * v.y
            # Убедимся, что классификация работает
            is_good = Edge.is_good_point(v)
            if is_good:
                assert 1.0 < r_squared < 4.0
            else:
                assert r_squared <= 1.0 or r_squared >= 4.0

    def test_box_polyedr(self):
        """Тест с полиэдром box.geom"""
        p = Polyedr("data/box.geom")
        result = p.good_edges_length_sum()
        assert result >= 0.0
