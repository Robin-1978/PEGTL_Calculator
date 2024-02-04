#pragma once
namespace calc::node{

struct basic_node{
    virtual ~basic_node() = default;
};


struct number : basic_node{
    double value;
};

struct plus  : node{

};

struct minus  : node{

};

struct multiply  : node{

};

struct divide  : node{
    
};





}