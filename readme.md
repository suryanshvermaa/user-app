## Creating new project in drogon
```
drogon_ctl create project user-app
```

## run server through docker compose
### build image
```
docker build -t user-app .
```
### run image
```
docker-compose up -d
```
### stop running image
```
docker-compose down
```