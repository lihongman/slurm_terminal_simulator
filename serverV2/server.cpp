#include "httplib.h"
#include "workflow_manager.h"

#include <chrono>
#include <cstdio>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>
#include <wrench.h>


// Define a long function which is used multiple times
#define get_time() (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count())

using httplib::Request;
using httplib::Response;
using json = nlohmann::json;

/**
 * @brief 
 */
httplib::Server server;

/**
 * @brief 
 */
time_t time_start = 0;

/**
 * @brief
 */
wrench::Simulation simulation;

/**
 * @brief
 */
wrench::Workflow workflow;

/**
 * @brief
 */
std::shared_ptr<wrench::WorkflowManager> wms;

// TEST PATHS

/**
 * @brief 
 * @param req 
 * @param res 
 */
void testGet(const Request& req, Response& res)
{
    std::printf("Path: %s\n\n", req.path.c_str());

    res.set_header("access-control-allow-origin", "*");
    res.set_content("Hello World!", "text/plain");
}

/**
 * @brief 
 * @param req 
 * @param res 
 */
void testPost(const Request& req, Response& res)
{
    std::printf("Path: %s\nBody: %s\n\n", req.path.c_str(), req.body.c_str());

    res.set_header("access-control-allow-origin", "*");
    res.set_content("Hello World!", "text/plain");
}


// GET PATHS

/**
 * @brief 
 * @param req 
 * @param res 
 */
void getTime(const Request& req, Response& res)
{
    std::printf("Path: %s\n\n", req.path.c_str());

    json body;

    if (time_start == 0)
    {
        res.status = 400;
        return;
    }

    body["time"] = get_time() - time_start;
    res.set_header("access-control-allow-origin", "*");
    res.set_content(body.dump(), "application/json");
}

/**
 * @brief
 * @param req
 * @param res
 */
void getQuery(const Request& req, Response& res)
{
    std::printf("Path: %s\n\n", req.path.c_str());

    json body;

    body["time"] = get_time() - time_start;
    body["query"] = "A query to the server was made.";
    res.set_header("access-control-allow-origin", "*");
    res.set_content(body.dump(), "application/json");
}

// POST PATHS

/**
 * @brief 
 * @param req 
 * @param res 
 */
void start(const Request& req, Response& res)
{
    std::printf("Path: %s\nBody: %s\n\n", req.path.c_str(), req.body.c_str());

    time_start = get_time();
    res.set_header("access-control-allow-origin", "*");
    //res.set_content("", "application/json");
}

/**
 * @brief 
 * @param req 
 * @param res 
 */
void add1(const Request& req, Response& res)
{
    std::printf("Path: %s\nBody: %s\n\n", req.path.c_str(), req.body.c_str());

    json body;
    time_start -= 60000;
    body["time"] = get_time() - time_start;
    res.set_header("access-control-allow-origin", "*");
    res.set_content(body.dump(), "application/json");
}

/**
 * @brief 
 * @param req 
 * @param res 
 */
void add10(const Request& req, Response& res)
{
    std::printf("Path: %s\nBody: %s\n\n", req.path.c_str(), req.body.c_str());

    json body;
    time_start -= 60000 * 10;
    body["time"] = get_time() - time_start;
    res.set_header("access-control-allow-origin", "*");
    res.set_content(body.dump(), "application/json");
}

/**
 * @brief 
 * @param req 
 * @param res 
 */
void add60(const Request& req, Response& res)
{
    std::printf("Path: %s\nBody: %s\n\n", req.path.c_str(), req.body.c_str());

    json body;
    time_start -= 60000 * 60;
    body["time"] = get_time() - time_start;
    res.set_header("access-control-allow-origin", "*");
    res.set_content(body.dump(), "application/json");
}

void addTask(const Request& req, Response& res)
{
    json req_body = req.body;
    std::printf("Path: %s\nBody: %s\n\n", req.path.c_str(), req.body.c_str());
    std::string file = req_body["file"].get<std::string>();
    wms->addJob(file);

    json body;
    body["time"] = get_time() - time_start;
    body["success"] = true;
    res.set_header("access-control-allow-origin", "*");
    res.set_content(body.dump(), "application/json");
}

// ERROR HANDLING

/**
 * @brief 
 * @param req 
 * @param res 
 */
void error_handling(const Request& req, Response& res)
{
    std::printf("%d\n", res.status);
}

/**
 * @brief 
 * @return 
 */
int main(int argc, char **argv)
{
    int port_number = 8080;
    // XML config file copied from batch-bag-of-tasks example
    std::string simgrid_config = "../four_hosts.xml";

    // Initialize WRENCH
    simulation.init(&argc, argv);
    simulation.instantiatePlatform(simgrid_config);
    std::vector<std::string> nodes = {"Node1", "Node2"};
    auto storage_service = simulation.add(new wrench::SimpleStorageService(
        "mainstorage", {"/"}, {{wrench::SimpleStorageServiceProperty::BUFFER_SIZE, "10000000"}}, {}));
    auto batch_service = simulation.add(new wrench::BatchComputeService("mainbatch", nodes, {}, {}));
    wms = simulation.add(new wrench::WorkflowManager({batch_service}, {storage_service}, "WMS"));

    // Add workflow to wms
    wms->addWorkflow(&workflow);

    // Handle GET requests
    server.Get("/", testGet);
    server.Get("/time", getTime);
    server.Get("/query", getQuery);

    // Handle POST requests
    server.Post("/", testPost);
    server.Post("/start", start);
    server.Post("/add1", add1);
    server.Post("/add10", add10);
    server.Post("/add60", add60);

    std::printf("Listening on port: %d\n", port_number);

    server.set_error_handler(error_handling);
    server.listen("localhost", port_number);
    return 0;
}