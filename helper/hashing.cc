#include "bcrypt/BCrypt.hpp"
#include<iostream>
using namespace std;

string hash_password(string password) {
    return BCrypt::generateHash(password,10);
}
bool isCorrectPassword(string password,string hashedPassword){
    return BCrypt::validatePassword(password,hashedPassword);
}