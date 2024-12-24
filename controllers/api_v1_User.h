#pragma once

#include <drogon/HttpController.h>
#include <drogon/orm/DbClient.h>


using namespace drogon;
using namespace drogon::orm;

namespace api
{
namespace v1
{
class User : public drogon::HttpController<User>
{
  public:
    User() : dbClient(app().getDbClient()) {}
    METHOD_LIST_BEGIN
    METHOD_ADD(User::signup, "/signup", Post);
    METHOD_ADD(User::login, "/login", Post);
    METHOD_ADD(User::auth, "/isAuthenticated", Post);
    METHOD_LIST_END

    void signup(const HttpRequestPtr& req, std::function<void (const HttpResponsePtr &)> &&callback);
    void login(const HttpRequestPtr& req, std::function<void (const HttpResponsePtr &)> &&callback);
    void auth(const HttpRequestPtr& req, std::function<void (const HttpResponsePtr &)> &&callback);
  private:
    DbClientPtr dbClient;
};
}
}
