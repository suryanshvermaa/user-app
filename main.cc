#include <drogon/drogon.h>
#include <cstdlib>
using namespace drogon;
int main() {
    try
    {   
        const char *port = std::getenv("PORT")?std::getenv("PORT"):"3000";
        app().addListener("0.0.0.0", std::stoi(port));
        app().loadConfigFile("../config.json");
        LOG_INFO<<"Server is running on port "<<port;
        app().run();
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }
    
    return 0;
}
