#include <catch2/catch_test_macros.hpp>
#include "graph.hpp"
#include <iostream>

SCENARIO( "Graph", "[inner_lib][graph]" )
{

    GIVEN("base graph functions")
    {
        Graph graph;

        WHEN("add vertexes")
        {
            graph.add_vertex(1);
            graph.add_vertex(2);
            graph.add_vertex(3);

            THEN("graph has 3 vertexes")
            {
                REQUIRE(graph.get_vertex_count() == 3);
                REQUIRE(graph.has_vertex(1));
                REQUIRE(graph.has_vertex(2));
                REQUIRE(!graph.has_edge(1, 2));
            }
        }

        WHEN("add edges")
        {
            graph.add_vertex(1);
            graph.add_vertex(2);
            graph.add_vertex(3);
            graph.add_edge(1, 2);
            graph.add_edge(2, 3);

            THEN("graph has 2 edges")
            {
                REQUIRE(graph.get_edge_count() == 2);
                REQUIRE(graph.has_edge(1, 2));
                REQUIRE(graph.has_edge(2, 3));
            }
        }

        WHEN("add cycle")
        {
            graph.add_vertex(1);
            graph.add_vertex(2);
            graph.add_vertex(3);
            graph.add_edge(1, 2);
            graph.add_edge(2, 3);
            graph.add_edge(3, 1);

            THEN("graph has cycle")
            {
                REQUIRE(get_return(graph.is_have_structure(1, CheckStructureCommand::CYCLE)) == true);
                REQUIRE(get_return(graph.is_have_structure(1, CheckStructureCommand::TREE)) == false);
            }
        }

        WHEN("add subgraph")
        {
            graph.add_vertex(1);
            graph.add_vertex(2);
            graph.add_vertex(3);
            graph.add_edge(1, 2);
            graph.add_edge(1, 3);
            graph.add_edge(2, 3);

            THEN("graph has subgraph")
            {
                REQUIRE(get_return(graph.is_have_structure(1, CheckStructureCommand::SUBGRAPH)) == true);
                REQUIRE(get_return(graph.is_have_structure(1, CheckStructureCommand::TREE)) == false);
            }
        }
    }

    GIVEN("first search")
    {
        Graph graph;
        /*
           1
          /|\
         2 3 4
          /|\
         5 6 7
          /|
         8 9
        */
        graph.add_edge(1, 2);
        graph.add_edge(1, 3);
        graph.add_edge(1, 4);

        graph.add_edge(3, 5);
        graph.add_edge(3, 6);
        graph.add_edge(3, 7);

        graph.add_edge(6, 8);
        graph.add_edge(6, 9);

        WHEN("skip at vertex 6") {
            bool is_skipped = false;
            int next_vertex = 0;
            auto err = graph.dfs(1, [&is_skipped, &next_vertex](int vertex, int, Colors, bool) -> CallbackCommand {
                std::cout << "vertex: " << vertex << std::endl;
                if (vertex == 6) {
                    is_skipped = true;
                    return CallbackCommand::SKIP_CHILDRENS_AND_NEXT_NEIGHBORS;
                }
                if (is_skipped) {
                    next_vertex = vertex;
                    return CallbackCommand::STOP;
                }
                return CallbackCommand::CONTINUE;
            });

            THEN("pass skipping") {
                REQUIRE(err.have_error() == 0);
                REQUIRE(is_skipped == true);
                REQUIRE(next_vertex == 4);
            }
        }

        WHEN("get shortest path") {
            /*
               1
              /|\
             2 3 4
              /|\  \
             5 6 7  |
              /|\  /
             8 9 10
            */
            graph.add_edge(6, 10);
            graph.add_edge(4, 10);
            std::vector<int> path;
            auto err = graph.get_shortest_path(1, 10, path);

            THEN("get shortest path") {
                REQUIRE(!is_error_code(err));
                REQUIRE(path == std::vector<int>({1, 4, 10}));
            }
        }
    }
}
