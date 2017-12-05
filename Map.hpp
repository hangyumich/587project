//
//  map.hpp
//  Sokoban
//
//  Created by Luyao Yuan on 2016/3/20.
//  Copyright © 2016年 Luyao Yuan. All rights reserved.
//
#ifndef MAP_HPP
#define MAP_HPP

#include <iostream>
#include <vector>
#include <assert.h>
#include <utility>
#include "types.hpp"
#include "variables.hpp"
#include <cmath>

using namespace std;

// An object representing a predicate
// Predicates have at least one argument, and at most two
// Unused arguments are always -1.
struct Predicate
{
    int arg_[3];
    Predicates type_;
    
    Predicate() {};
    
    Predicate(Predicates t, int arg1, int arg2 = -1, int arg3 = -1)
    {
        type_ = t;
        arg_[0] = arg1;
        arg_[1] = arg2;
        arg_[2] = arg3;
    }
    
    bool operator==(const Predicate& p) const
    {
        return (type_ == p.type_) && (arg_[0] == p.arg_[0]) && (arg_[1] == arg_[1]) && (arg_[2] == arg_[2]);
    }
};

class Map
{
    vector<vector<MapType>> SokobanMap_;
    unsigned int width_;
    unsigned int height_;
    bool initiated_;

public:
    //constructor
    Map();
    Map(unsigned h, unsigned w);
    Map(vector<vector<MapType>>&);
    
    //getters
    unsigned getWidth() const;
    unsigned getHeight() const;
    bool getMapStatus() const;
    vector<vector<MapType>> getMap() const;
    MapType getPosition(unsigned, unsigned w = -1) const;
    vector<Predicate> map2Predicates(bool print = true) const;
    vector<float> map2Momentum(vector<Predicate>&, bool print = true) const;
    
    //setters
    bool setWidth(unsigned);
    bool setHeight(unsigned);
    bool setPosition(unsigned, unsigned, MapType);
    void finishMap();
    
    //utilize functions
    unsigned cor2ind(unsigned, unsigned) const;
    pair<unsigned, unsigned> ind2cor(unsigned) const;
    int dist(unsigned, unsigned) const;
    
};

//print map out
ostream& operator<<(ostream& os, const Map&);
#endif