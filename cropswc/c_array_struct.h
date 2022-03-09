//
// Created by 黄磊 on 27.12.21.
//

#ifndef UNTITLED_C_ARRAY_STRUCT_H
#define UNTITLED_C_ARRAY_STRUCT_H

template<class ELT, int SIZE>
class c_array {
public:
    typedef ELT value_type;
    typedef int size_type;
    typedef int index_type;
    typedef ELT* iterator;
    typedef ELT const * const_iterator;

    // indexing operators are the most important thing to retain
    // Problem - gcc on Mac makes operator[] ambiguous when cast operator is included
    // const ELT& operator[](index_type i) const {return data[i];}
    // ELT& operator[](index_type i) {return data[i];}

    // allow implicit conversion to pointer
    // Hopefully this resolves index operator too.
    operator ELT*() {return data;}
    operator const ELT*() const {return data;}

    static size_type size() {return SIZE;}

    iterator begin() {return &data[0];}
    const_iterator begin() const {return &data[0];}
    iterator end() {return &data[SIZE];}
    const_iterator end() const {return &data[SIZE];}

private:
    ELT data[SIZE];
};

#endif //UNTITLED_C_ARRAY_STRUCT_H
