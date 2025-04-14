#include "graph.hpp"
#include <deque>
#include <stack>
#include <algorithm>

struct VertexData
{
    int vertex;
    int parent;
    bool is_backtracking;

    VertexData(int vertex, int parent, bool is_backtracking) : vertex(vertex), parent(parent), is_backtracking(is_backtracking) {}
};

Graph::EdgesIterator::EdgesIterator(Edges::iterator it) : it_(it)
{}

int Graph::EdgesIterator::operator*()
{
    return *it_;
}

Graph::EdgesIterator& Graph::EdgesIterator::operator++()
{
    ++it_;
    return *this;
}

bool Graph::EdgesIterator::operator!=(const EdgesIterator& other) const 
{ 
    return it_ != other.it_; 
}

Graph::Iterator::Iterator(GraphData::iterator it) : it_(it)
{}

std::pair<int, Graph::EdgesIterator> Graph::Iterator::operator*() 
{
    return std::make_pair(it_->first, EdgesIterator(it_->second.begin()));
}

Graph::Iterator& Graph::Iterator::operator++() 
{
    ++it_;
    return *this;
}

bool Graph::Iterator::operator!=(const Iterator& other) const 
{ 
    return it_ != other.it_; 
}

Graph::Iterator Graph::begin() {
    return Iterator(adj_list_.begin());
}

Graph::Iterator Graph::end() {
    return Iterator(adj_list_.end());
}

void Graph::add_vertex(int vertex)
{
    adj_list_[vertex];
}

void Graph::remove_vertex(int vertex)
{
    auto vertex_it = adj_list_.find(vertex);
    if (vertex_it == adj_list_.end())
        return;

    edge_count_ -= vertex_it->second.size();

    adj_list_.erase(vertex);

    for (auto it = adj_list_.begin(); it != adj_list_.end(); ++it)
        if (it->second.find(vertex) != it->second.end()) {
            edge_count_--;
            it->second.erase(vertex);
        }
}

void Graph::add_edge(int from_vertex, int to_vertex, bool add_reverse_edge)
{
    adj_list_[to_vertex];
    
    if (adj_list_[from_vertex].find(to_vertex) == adj_list_[from_vertex].end())
        edge_count_++;

    adj_list_[from_vertex].insert(to_vertex);

    if (add_reverse_edge) {
        if (adj_list_[to_vertex].find(from_vertex) == adj_list_[to_vertex].end())
            edge_count_++;

        adj_list_[to_vertex].insert(from_vertex);
    }
}

void Graph::remove_edge(int from_vertex, int to_vertex, bool remove_reverse_edge)
{
    if (adj_list_[from_vertex].find(to_vertex) != adj_list_[from_vertex].end())
        edge_count_--;

    adj_list_[from_vertex].erase(to_vertex);

    if (remove_reverse_edge) {

        if (adj_list_[to_vertex].find(from_vertex) != adj_list_[to_vertex].end())
            edge_count_--;
            
        adj_list_[to_vertex].erase(from_vertex);
    }
}

bool Graph::has_vertex(int vertex) const
{
    return adj_list_.find(vertex) != adj_list_.end();
}

bool Graph::has_edge(int from_vertex, int to_vertex, bool check_reverse_edge)
{
    return adj_list_[from_vertex].find(to_vertex) != adj_list_[from_vertex].end() 
    && (!check_reverse_edge 
    || (check_reverse_edge && adj_list_[to_vertex].find(from_vertex) != adj_list_[to_vertex].end()));
}

std::size_t Graph::get_vertex_count() const 
{ 
    return adj_list_.size(); 
}

std::size_t Graph::get_edge_count() const 
{ 
    return edge_count_; 
}

ErrorCode Graph::dfs(
    int vertex, 
    std::function<CallbackCommand(int, int, Colors, bool)> callback, bool handle_backtracking) const
{
    std::stack<VertexData> traversal_vertexes;

    std::unordered_map<int, bool> visited_vertexes;
    visited_vertexes.reserve(adj_list_.size());

    traversal_vertexes.emplace(vertex, vertex, false);
   
    while (!traversal_vertexes.empty()) {

        VertexData vertex_data = traversal_vertexes.top();
        traversal_vertexes.pop();

        Colors color = Colors::WHITE;

        if (vertex_data.is_backtracking) {
            visited_vertexes[vertex_data.vertex] = false;
            color = Colors::BLACK;
            if (!handle_backtracking)
                continue;

        } else if (visited_vertexes.find(vertex_data.vertex) == visited_vertexes.end()) {   // white

            visited_vertexes[vertex_data.vertex] = true;
            traversal_vertexes.emplace(vertex_data.vertex, vertex_data.parent, true);

            auto neighbors = adj_list_.find(vertex_data.vertex);
            if (neighbors == adj_list_.end())
                return ErrorCode(2, -1);

            for (auto neighbor : neighbors->second)
                traversal_vertexes.emplace(neighbor, vertex_data.vertex, false);
        } else if (visited_vertexes[vertex_data.vertex]) {   // gray
            color = Colors::GRAY;
        } else {   // black
            color = Colors::BLACK;
        }

        CallbackCommand command = callback(vertex_data.vertex, vertex_data.parent, color, vertex_data.is_backtracking);
        if (command == STOP)
            break;

        if (!vertex_data.is_backtracking && (command == SKIP_CHILDRENS || command == SKIP_CHILDRENS_AND_NEXT_NEIGHBORS)) {
            while (!traversal_vertexes.top().is_backtracking)
                traversal_vertexes.pop();
            visited_vertexes[traversal_vertexes.top().vertex] = false;
            traversal_vertexes.pop();
        }

        if (command == SKIP_NEXT_NEIGHBORS || command == SKIP_CHILDRENS_AND_NEXT_NEIGHBORS) {
            while (!traversal_vertexes.top().is_backtracking)
                traversal_vertexes.pop();
            visited_vertexes[traversal_vertexes.top().vertex] = false;
            traversal_vertexes.pop();
        }
    }

    return ErrorCode(0, 0);
}

ErrorCode Graph::bfs(int vertex, std::function<CallbackCommand(int, int)> callback) const
{
    std::deque<VertexData> traversal_vertexes;
    std::unordered_set<int> visited_vertexes;

    traversal_vertexes.emplace_back(vertex, vertex, false);

    while (!traversal_vertexes.empty()) {

        VertexData vertex_data = traversal_vertexes.front();
        traversal_vertexes.pop_front();

        if (vertex_data.is_backtracking)
            continue;

        if (visited_vertexes.find(vertex_data.vertex) != visited_vertexes.end())
            continue;

        visited_vertexes.insert(vertex_data.vertex);

        auto neighbors = adj_list_.find(vertex_data.vertex);
        if (neighbors == adj_list_.end())
            return ErrorCode(2, -1);

        for (auto neighbor : neighbors->second) {
            if (visited_vertexes.find(neighbor) == visited_vertexes.end()) {
                traversal_vertexes.emplace_back(neighbor, vertex_data.vertex, false);
            }
        }

        traversal_vertexes.emplace_back(vertex_data.vertex, vertex_data.parent, true);

        CallbackCommand command = callback(vertex_data.vertex, vertex_data.parent);
        if (command == STOP)
            break;

        if (command == SKIP_CHILDRENS || command == SKIP_CHILDRENS_AND_NEXT_NEIGHBORS) {
            traversal_vertexes.pop_back();
            while (!traversal_vertexes.back().is_backtracking)
                traversal_vertexes.pop_back();
        }

        if (command == SKIP_NEXT_NEIGHBORS || command == SKIP_CHILDRENS_AND_NEXT_NEIGHBORS) {
            while (!traversal_vertexes.front().is_backtracking)
                traversal_vertexes.pop_front();
            traversal_vertexes.pop_front();
        }
    }

    return ErrorCode(0, 0);
}

ReturnOrErrorCode<bool> Graph::is_have_structure(int vertex, CheckStructureCommand structure) const
{
    bool is_have_structure = false;
    auto err = dfs(vertex, [&is_have_structure, structure](int vertex, bool parent, Colors color, bool) {

        if ((structure == SUBGRAPH && color != Colors::BLACK)
            || (structure == CYCLE && color != Colors::GRAY)
            || (structure == NONE && color == Colors::WHITE)) {
            is_have_structure = true;
            return CallbackCommand::STOP;
        }
        return CallbackCommand::CONTINUE;
    });

    if (err.have_error() != 0)
        return err;

    return is_have_structure;
}

ReturnOrErrorCode<bool> Graph::get_shortest_path(int vertex_from, int vertex_to, std::vector<int>& path)
{
    Graph graph;

    bool have_path = false;
    auto err = bfs(vertex_from, [&have_path, vertex_to, &path, &graph](int vertex, int parent) {

        graph.add_edge(vertex, parent);

        if (vertex == vertex_to) {
            have_path = true;
            return CallbackCommand::STOP;
        }

        return CallbackCommand::CONTINUE;
    });

    if (err.have_error() != 0)
        return err;

    if (!have_path)
        return false;

    std::size_t cur_count = path.size();

    graph.dfs(vertex_to, [vertex_from, &path](int vertex, int parent, Colors, bool) {

        path.push_back(vertex);

        if (vertex == vertex_from)
            return CallbackCommand::STOP;

        return CallbackCommand::CONTINUE;
    });

    std::reverse(path.begin() + cur_count, path.end());

    return true;
}

ReturnOrErrorCode<int> Graph::get_free_vertex_id()
{
    if (get_vertex_count() >= INT_MAX)
        return ErrorCode{2, -2};

    int next_vertex_id = static_cast<int>(get_vertex_count());
        
    while (has_vertex(next_vertex_id)) {
        next_vertex_id++;
        if (next_vertex_id == INT_MAX)
            next_vertex_id = 0;
    }

    return next_vertex_id;
}

std::vector<int> Graph::get_pointing_vertexes(int vertex) const
{
    std::vector<int> pointing_vertexes;
    for (auto pointing_vertex : adj_list_)
        if (pointing_vertex.second.find(vertex) != pointing_vertex.second.end())
            pointing_vertexes.push_back(pointing_vertex.first);
    return pointing_vertexes;   
}
