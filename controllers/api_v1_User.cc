#include "api_v1_User.h"
#include <drogon/orm/Mapper.h>
#include <drogon/drogon.h>
#include<iostream>
#include<helper/hashing.cc>
#include<helper/jwt.cc>
using namespace api::v1;

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

void User::auth(const HttpRequestPtr& req, std::function<void (const HttpResponsePtr &)> &&callback){
    auto body=req->getJsonObject();
    string token=(*body)["token"].as<string>();
    string decoded=decoded_token(token);
    Json::Value obj;
    obj["success"]=true;
    LOG_INFO<<"id is:"<<decoded;
    auto res=HttpResponse::newHttpJsonResponse(obj);
    res->setStatusCode(k200OK);
    callback(res);
}

