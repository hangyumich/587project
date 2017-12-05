//
//  types.hpp
//  Sokoban
//
//  Created by Luyao Yuan on 2016/3/20.
//  Copyright © 2016年 Luyao Yuan. All rights reserved.
//

#pragma once

#include <unordered_map>
#include <string>
using namespace std;

enum MapType
{
    ROBOT,
    BOX,
    OBSTACLE,
    FREE
};

const unordered_map<char, int> Symbol2MapType =
{
    {'@', ROBOT},
    {'$', BOX},
    {'*', OBSTACLE},
    {' ', FREE}
};

const unordered_map<int, char> MapType2Symbol =
{
    {ROBOT, '@'},
    {BOX, '$'},
    {OBSTACLE, '*'},
    {FREE, ' ' }
};

enum Predicates
{
    Linear,
    Next,
    Obstacle,
    Box,
    Empty,
    Robot
};

const unordered_map<string, int> Name2Predicate =
{
    {"box", Box},
    {"robot", Robot},
    {"obstacle", Obstacle},
    {"empty", Empty},
    {"linear", Linear},
    {"next", Next}
};

const unordered_map<int, string> Predicate2Name =
{
    {Box, "box"},
    {Robot, "robot"},
    {Obstacle, "obstacle"},
    {Empty, "empty"},
    {Next, "next"},
    {Linear, "linear"}
};

enum Actions
{
    START,
    FINISH,
    GO,
    PUSH
};

const unordered_map<string, int> Name2Action =
{
    {"start", START},
    {"finish", FINISH},
    {"go", GO},
    {"push", PUSH}
};

const unordered_map<int, string> Action2Name =
{
    {START, "start"},
    {FINISH, "finish"},
    {GO, "go"},
    {PUSH, "push"}
};
