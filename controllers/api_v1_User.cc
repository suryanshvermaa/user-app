#include "api_v1_User.h"
#include <drogon/orm/Mapper.h>
#include <drogon/drogon.h>
#include<iostream>
#include<helper/hashing.cc>
#include<helper/jwt.cc>
using namespace api::v1;

//signup controller
void User::signup(const HttpRequestPtr& req, std::function<void (const HttpResponsePtr &)> &&callback){
    auto userData=req->getJsonObject();
     if(!userData || !userData->isMember("email") || !userData->isMember("password") || !userData->isMember("name")) {
        auto res=drogon::HttpResponse::newHttpResponse();
        res->setStatusCode(drogon::k400BadRequest);
        res->setBody("All fields are required");
        callback(res);
        return;
    }
    std::string nameStr=(*userData)["name"].asString();
    std::string emailStr=(*userData)["email"].asString();
    std::string passwordStr=(*userData)["password"].asString();

    dbClient->execSqlAsync(
        "INSERT INTO users (name, email,password) VALUES ($1, $2, $3) returning id",
        [callback,emailStr](const Result& result) {
             Json::Value resObj;
             resObj["message"]="user created successfully";
             resObj["id"]=result[0]["id"].as<int>();
             resObj["token"]=generate_token(emailStr,result[0]["id"].as<string>());
             auto response = HttpResponse::newHttpJsonResponse(resObj);
            response->setStatusCode(k200OK);
            callback(response);
        },
        [callback](const DrogonDbException& e) {
            auto response = HttpResponse::newHttpJsonResponse(Json::Value(e.base().what()));
            response->setStatusCode(k500InternalServerError);
            callback(response);
        },
        nameStr,emailStr,hash_password(passwordStr));
}


//login controller
void User::login(const HttpRequestPtr& req, std::function<void (const HttpResponsePtr &)> &&callback){
    auto userData=req->getJsonObject();
    if(!userData || !userData->isMember("email") || !userData->isMember("password")){
        auto res=drogon::HttpResponse::newHttpResponse();
        res->setStatusCode(drogon::k400BadRequest);
        res->setBody("All fields are required");
        callback(res);
        return;
    }
    std::string emailStr=(*userData)["email"].asString();
    std::string passwordStr=(*userData)["password"].asString();
    dbClient->execSqlAsync(
        "SELECT * from users where email=$1",
        [callback,passwordStr,emailStr](const Result& result) {
            if(result[0]["id"].as<string>()==""){
                Json::Value user;
                user["error"]="user not found",
                user["success"]=false;
                auto response = HttpResponse::newHttpJsonResponse(user);
                response->setStatusCode(k500InternalServerError);
                callback(response);
                return;
            }
            if(isCorrectPassword(passwordStr,result[0]["password"].as<string>())){
                Json::Value user;
                user["token"]=generate_token(emailStr,result[0]["id"].as<string>());
                user["success"]=true;
                user["message"]="user login successful";
                auto response = HttpResponse::newHttpJsonResponse(user);
                response->setStatusCode(k200OK);
                callback(response);
                return;
            }else{
                Json::Value user;
                user["error"]="Incorrect email or password",
                user["success"]=false;
                auto response = HttpResponse::newHttpJsonResponse(user);
                response->setStatusCode(k500InternalServerError);
                callback(response);
            }
        },
        [callback](const DrogonDbException& e) {
            auto response = HttpResponse::newHttpJsonResponse(Json::Value(e.base().what()));
            response->setStatusCode(k500InternalServerError);
            callback(response);
        },
        emailStr);
}

//get users
void User::getUsers(const HttpRequestPtr& req, std::function<void (const HttpResponsePtr &)> &&callback){
    dbClient->execSqlAsync("SELECT * from users",
        [callback](const Result& result) {
            Json::Value respObj;
            Json::Value users;
            for(auto row:result){
                Json::Value user;
                user["id"]=row["id"].as<int>();
                user["name"]=row["name"].as<string>();
                user["email"]=row["email"].as<string>();
                users.append(user);
            }
            respObj["users"]=users;
            respObj["success"]=true;
            respObj["message"]="users fetched successfully";
            auto response = HttpResponse::newHttpJsonResponse(respObj);
            response->setStatusCode(k200OK);
            callback(response); 
        },  
        [callback](const DrogonDbException& e) {
            auto response = HttpResponse::newHttpJsonResponse(Json::Value(e.base().what()));
            response->setStatusCode(k500InternalServerError);
            callback(response);
        });
}

// //update user
void User::updateUser(const HttpRequestPtr& req, std::function<void (const HttpResponsePtr &)> &&callback){
    auto userData=req->getJsonObject();
    if(!userData || !userData->isMember("token")|| !userData->isMember("password") || !userData->isMember("name")){
        auto res=drogon::HttpResponse::newHttpResponse();
        res->setStatusCode(drogon::k400BadRequest);
        res->setBody("Not authorized or all fields are required");
        callback(res);
        return;
    }
    string token=(*userData)["token"].asString();
    string decoded=decoded_token(token);
    if(decoded==""){
        auto res=drogon::HttpResponse::newHttpResponse();
        res->setStatusCode(drogon::k400BadRequest);
        res->setBody("Not authorized");
        callback(res);
        return;
    }
    int id=stoi(decoded);
    string password=(*userData)["password"].asString();
    string name=(*userData)["name"].asString();
    if(password!=""&&name!=""){
        dbClient->execSqlAsync(
            "UPDATE users SET name=$1,password=$2 WHERE id=$3",
            [callback](const Result& result) {
                Json::Value resObj;
                resObj["message"]="user updated successfully";
                auto response = HttpResponse::newHttpJsonResponse(resObj);
                response->setStatusCode(k200OK);
                callback(response);
            },
            [callback](const DrogonDbException& e) {
                auto response = HttpResponse::newHttpJsonResponse(Json::Value(e.base().what()));
                response->setStatusCode(k500InternalServerError);
                callback(response);
            },
            name,hash_password(password),id);
    }
}


// //delete user

void User::deleteUser(const HttpRequestPtr& req, std::function<void (const HttpResponsePtr &)> &&callback){
    auto userTokenData=req->getJsonObject();
    if(!userTokenData || !userTokenData->isMember("token")){
        auto res=drogon::HttpResponse::newHttpResponse();
        res->setStatusCode(drogon::k400BadRequest);
        res->setBody("Not authorized");
        callback(res);
        return;
    }
    string token=(*userTokenData)["token"].asString();
    string decoded=decoded_token(token);
    if(decoded==""){
        auto res=drogon::HttpResponse::newHttpResponse();
        res->setStatusCode(drogon::k400BadRequest);
        res->setBody("Not authorized");
        callback(res);
        return;
    }
    int id=stoi(decoded);
    dbClient->execSqlAsync(
        "DELETE FROM users WHERE id=$1",
        [callback](const Result& result) {
            Json::Value resObj;
            resObj["message"]="user deleted successfully";
            auto response = HttpResponse::newHttpJsonResponse(resObj);
            response->setStatusCode(k200OK);
            callback(response);
        },
        [callback](const DrogonDbException& e) {
            auto response = HttpResponse::newHttpJsonResponse(Json::Value(e.base().what()));
            response->setStatusCode(k500InternalServerError);
            callback(response);
        },
        id);
}