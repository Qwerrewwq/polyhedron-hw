#ifndef SEGMENT_H
#define SEGMENT_H

/* One-dimensional segment */
typedef struct {
    double beg, fin;
} Segment;

/* Constructor */
static inline Segment segment_create(double beg, double fin) {
    Segment s = {beg, fin};
    return s;
}

/* Is the segment degenerate? (beg >= fin) */
static inline int segment_is_degenerate(Segment s) {
    return s.beg >= s.fin;
}

/* Intersection with another segment (modifies this segment in place) */
static inline void segment_intersect(Segment *s, Segment other) {
    if (other.beg > s->beg) {
        s->beg = other.beg;
    }
    if (other.fin < s->fin) {
        s->fin = other.fin;
    }
}

/* Subtraction: returns array of 2 segments [left, right] */
/* The difference of two segments always produces exactly two segments */
static inline void segment_subtraction(Segment s, Segment other, Segment result[2]) {
    /* Left segment: [s.beg, min(s.fin, other.beg)] */
    double left_fin = (s.fin < other.beg) ? s.fin : other.beg;
    result[0] = segment_create(s.beg, left_fin);
    
    /* Right segment: [max(s.beg, other.fin), s.fin] */
    double right_beg = (s.beg > other.fin) ? s.beg : other.fin;
    result[1] = segment_create(right_beg, s.fin);
}

#endif /* SEGMENT_H */
