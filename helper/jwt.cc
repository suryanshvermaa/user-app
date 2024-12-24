#include<jwt-cpp/jwt.h>
#include<iostream>
#include<string>
using namespace std;

const std::string secret="njwjnkfhkuwgyq57ggvds67dbehjdkjdiu";
std::string generate_token(std::string email,string id) {
    return jwt::create()
        .set_issuer("auth0")
        .set_type("JWS")
        .set_payload_claim("email", jwt::claim(email))
        .set_payload_claim("id", jwt::claim(id))
        .set_expires_at(std::chrono::system_clock::now() + std::chrono::hours(1))
        .sign(jwt::algorithm::hs256{secret});
}

string decoded_token(string token){
    try
    {
        auto decoded=jwt::decode(token);
        auto verifier = jwt::verify()
        .with_issuer("auth0")
        .with_type("JWS")
        .allow_algorithm(jwt::algorithm::hs256{secret});
        verifier.verify(decoded);
        for(auto& e : decoded.get_payload_json()){
            if(e.first=="id") {
                return e.second.to_str();
            }
        }
        return "";
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
        return "";
    }
}