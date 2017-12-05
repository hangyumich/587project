//
//  map.cpp
//  Sokoban
//
//  Created by Luyao Yuan on 2016/3/20.
//  Copyright © 2016年 Luyao Yuan. All rights reserved.
//

#include "Map.hpp"
Map::Map()
:width_(5), height_(5), initiated_(false)
{
    SokobanMap_.resize(height_, vector<MapType>(width_, FREE));
}

Map::Map(unsigned h, unsigned w)
:width_(w), height_(h), initiated_(false)
{
    SokobanMap_.resize(height_, vector<MapType>(width_, FREE));
}

Map::Map(vector<vector<MapType>>& map)
{
    width_ = unsigned(map[0].size());
    height_ = unsigned(map.size());
    SokobanMap_ = map;
}

unsigned Map::getWidth() const
{
    return width_;
}

unsigned Map::getHeight() const
{
    return height_;
}

bool Map::getMapStatus() const
{
    return initiated_;
}

vector<vector<MapType>> Map::getMap() const
{
    return SokobanMap_;
}

MapType Map::getPosition(unsigned h, unsigned w)const
{
    if(w == -1)
    {
        auto temp = ind2cor(h);
        h = temp.first;
        w = temp.second;
    }
    assert(w < width_ && h < height_);
    return SokobanMap_[h][w];
}

bool Map::setWidth(unsigned width)
{
    if(initiated_)
    {
        cerr << "map already built\n";
        return false;
    }
    width_ = width;
    return true;
}


bool Map::setHeight(unsigned height)
{
    if(initiated_)
    {
        cerr << "map already built\n";
        return false;
    }
    height_ = height;
    return true;
}

bool Map::setPosition(unsigned height, unsigned width, MapType type)
{
    if(initiated_)
    {
        cerr << "map already built\n";
        return false;
    }
    assert(height < height_);
    assert(width < width_);
    SokobanMap_[height][width] = type;
    return true;
}

void Map::finishMap()
{
    initiated_ = true;
}

unsigned Map::cor2ind(unsigned h, unsigned w) const
{
    assert(h < height_);
    assert(w < width_);
    return h * width_ + w;
}

pair<unsigned, unsigned> Map::ind2cor(unsigned index) const
{
    assert(index < width_ * height_);
    unsigned w = index % width_;
    unsigned h = index / width_;
    return make_pair(h, w);
}

ostream& operator<<(ostream& os, const Map& m)
{
    auto map = m.getMap();
    for(unsigned i = 0; i < map.size(); ++i)
    {
        //os << "|";
        for(auto grid: map[map.size() - 1 - i])
        {
            os << MapType2Symbol.at(grid);
            os << " ";
        }
        os << endl;
    }
    return os;
}

vector<Predicate> Map::map2Predicates(bool print) const
{
    if(!initiated_)
    {
        cerr << "not a full map yet, don't believe in what I tell you now. \n";
    }
    vector<Predicate> initials;
    unsigned index = 0;
    for(auto line: SokobanMap_)
    {
        for(auto grid: line)
        {
            Predicate temp;
            switch(grid)
            {
                case ROBOT:
                    temp = Predicate(Robot, index);
                    index++;
                    break;
                case BOX:
                    temp = Predicate(Box, index);
                    index++;
                    break;
                case FREE:
                    temp = Predicate(Empty, index);
                    index++;
                    break;
                case OBSTACLE:
                    //don't need that one
                    //temp = Predicate(Obstacle, index);
                    index++;
                    continue;
                    break;
            }
            initials.push_back(temp);
            if(temp.type_ == Robot)
                initials.push_back(Predicate(Empty, index - 1));
        }
    }
    assert(index == width_ * height_);
    //spatial information of the map
    for(unsigned i = 0; i < height_; ++i)
    {
        for(unsigned j = 0; j < width_ - 1; ++j)
        {
            if(SokobanMap_[i][j] != OBSTACLE && SokobanMap_[i][j + 1] != OBSTACLE)
                initials.push_back(Predicate(Next, cor2ind(i, j), cor2ind(i, j + 1)));
        }
    }
    for(unsigned i = 0; i < height_ - 1; ++i)
    {
        for(unsigned j = 0; j < width_; ++j)
        {
            if(SokobanMap_[i][j] != OBSTACLE && SokobanMap_[i + 1][j] != OBSTACLE)
                initials.push_back(Predicate(Next, cor2ind(i, j), cor2ind(i + 1, j)));
        }
    }
    for(unsigned i = 0; i < height_; ++i)
    {
        for(unsigned j = 0; j < width_ - 2; ++j)
        {
            if(SokobanMap_[i][j] != OBSTACLE && SokobanMap_[i][j + 1] != OBSTACLE && SokobanMap_[i][j + 2] != OBSTACLE)
                initials.push_back(Predicate(Linear, cor2ind(i, j), cor2ind(i, j + 1), cor2ind(i, j + 2)));
        }
    }
    for(unsigned i = 0; i < height_ - 2; ++i)
    {
        for(unsigned j = 0; j < width_; ++j)
        {
            if(SokobanMap_[i][j] != OBSTACLE && SokobanMap_[i + 1][j] != OBSTACLE && SokobanMap_[i + 2][j] != OBSTACLE)
                initials.push_back(Predicate(Linear, cor2ind(i, j), cor2ind(i + 1, j), cor2ind(i + 2, j)));
        }
    }
    if(print)
    {
        for(auto predicate: initials)
        {
            cout << Predicate2Name.at(predicate.type_) << " ";
            for(int i = 0; i < 3; ++i)
            {
                if(predicate.arg_[i] == -1)
                    break;
                auto gridL = ind2cor(predicate.arg_[i]);
                cout << gridL.first << " " << gridL.second << " ;";
            }
            cout << endl;
        }
    }
    return initials;
}

int Map::dist(unsigned g1, unsigned g2) const
{
    auto p1 = ind2cor(g1);
    auto p2 = ind2cor(g2);
    return labs(int(p1.first) - int(p2.first)) + labs(int(p1.second) - int(p2.second));
}

vector<float> Map::map2Momentum(vector<Predicate>& goals, bool print) const
{
    vector<float> momentum;
    vector<int> boxes;
    //get locations of boxes
    for(unsigned i = 0; i < height_; ++i)
    {
        for(unsigned j = 0; j < width_; ++j)
        {
            if(SokobanMap_[i][j] == BOX)
                boxes.push_back(cor2ind(i, j));
        }
    }
    //calculate momentum
    for(unsigned i = 0; i < height_; ++i)
    {
        for(unsigned j = 0; j < width_; ++j)
        {
            float start = 0.05;
            auto temp = cor2ind(i, j);
            for(auto box: boxes)
                start -= dist(box, temp) * 0.001;
            for(auto goal: goals)
                if(goal.type_ != Linear && goal.type_ != Next)
                    start -= dist(goal.arg_[0], temp) * 0.001;
            momentum.push_back(start);
        }
    }
    assert(momentum.size() == width_ * height_);
    if(print)
    {
        for(unsigned i = 0; i < height_; ++i)
        {
            for(unsigned j = 0; j < width_; ++j)
            {
                cout << momentum[cor2ind(i, j)] << " ";
            }
            cout << endl;
        }
    }
    return momentum;
}






