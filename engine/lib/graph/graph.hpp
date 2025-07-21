#ifndef INCLUDE_GRAPH_HPP_
#define INCLUDE_GRAPH_HPP_

#include "error_code.hpp"

#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <functional>

/*
 * SKIP_CHILDRENS - delete from stack all children of vertex'
 * SKIP_NEXT_NEIGHBORS - delete from stack all next neighbors of vertex
 * SKIP_CHILDRENS_AND_NEXT_NEIGHBORS - delete from stack all childrens and next neighbors of vertex
 */
enum CallbackCommand {
    CONTINUE,
    SKIP_CHILDRENS,
    SKIP_CHILDRENS_AND_NEXT_NEIGHBORS,
    STOP
};

enum Colors {
    WHITE,  // vertex has not been discovered
    GRAY,   // vertex has been discovered but some of its descendents have not yet been discovered
    BLACK   // vertex and all of its descendents have been discovered
};

enum CheckStructureCommand {
    TREE,
    CYCLE,
    SUBGRAPH
};

class Graph {

    using Edges = std::unordered_set<int>;
    using GraphData = std::unordered_map<int, Edges>;

    GraphData adj_list_;
    std::size_t edge_count_ = 0;

public:
    void add_vertex(int vertex);
    void remove_vertex(int vertex);
    void add_edge(int from_vertex, int to_vertex, bool add_reverse_edge = false);  
    void remove_edge(int from_vertex, int to_vertex, bool remove_reverse_edge = false);
    
    bool has_vertex(int vertex) const;
    bool has_edge(int from_vertex, int to_vertex, bool check_reverse_edge = false);
    std::size_t get_vertex_count() const;
    std::size_t get_edge_count();
    std::size_t get_edge_count(int vertex);

    /* deep first search 
        callback - vertex, parent, color, is_backtracking
    */
    ErrorCode dfs(int vertex, std::function<CallbackCommand(int, int, Colors, bool)> callback, bool handle_backtracking = false) const;
    /* breadth first search 
        callback - vertex, parent 
    */
    ErrorCode bfs(int vertex, std::function<CallbackCommand(int, int)> callback) const;
    ReturnOrErrorCode<bool> is_have_structure(int vertex, CheckStructureCommand structure) const;
    ReturnOrErrorCode<bool> get_shortest_path(int vertex_from, int vertex_to, std::vector<int>& path);
    ReturnOrErrorCode<int> get_free_vertex_id();

    std::vector<int> get_pointing_vertexes(int vertex) const;

    class EdgesIterator {

        Edges::iterator it_;

    public:
        EdgesIterator(Edges::iterator it);
        int operator*();
        EdgesIterator& operator++();
        bool operator!=(const EdgesIterator& other) const;
    };

    class Iterator {

        GraphData::iterator it_;

    public:
        Iterator(GraphData::iterator it);
        std::pair<int, EdgesIterator> operator*();
        Iterator& operator++();
        bool operator!=(const Iterator& other) const;
    };

    Iterator begin();
    Iterator end();
};

#endif  // INCLUDE_GRAPH_HPP_