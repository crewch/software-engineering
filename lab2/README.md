# Домашнее задание 02

## Разработка REST API сервиса

### Вариант №21 — Система управления арендой автомобилей ([аналогичный](https://www.hertz.com/))

### Выполнил студент группы М8О-105СВ-25 Крючков Артемий Владимирович

## 1. Описание выбранного варианта

#### Приложение должно содержать следующие данные:

- Пользователь
- Автомобиль
- Аренда

#### Реализовать API:

- Создание нового пользователя
- Поиск пользователя по логину
- Поиск пользователя по маске имя и фамилии
- Добавление автомобиля в парк
- Получение списка доступных автомобилей
- Поиск автомобилей по классу
- Создание аренды
- Получение активных аренд пользователя
- Завершение аренды
- Получение истории аренд

## 2. Проектирование REST API

В файле [api/openapi_full.yaml](api/openapi_full.yaml) схема API. В директории [docs/definitions](docs/definitions) - модели данных.

---

## 3. Реализация REST API сервиса

- Реализовано на Userver;
- Реализованы 7 эндпоинтов;
- Соблюдены принципы Domain Driven Design;
- DTO сгенерировано chaotic.

---

## 4. Реализация аутентификации

- Реализована JWT авторизация (простая, HS256). Пристствует валидация юзера по id с помощью jwt на всех endpoint-ах, кроме создание пользователя и логин. Подробнее в [configs/static_config.yaml](configs/static_config.yaml)

---

## 5. Документирование API

Openapi-спецификация в [api/openapi_full.yaml](api/openapi_full.yaml)

---

## 6. Тестирование

Интеграционные тесты на python в директории tests. Для запуска используйте команды `make test-debug` или `make test-release`.

Также есть [конфиг postman](postman/-api-v1.postman_collection.json) для ручного тестирования.

---

## 7. Результат

- Исходный код: [src](src)
- openapi.yaml: [api/openapi_full.yaml](api/openapi_full.yaml)
- README.md: [README.md](README.md)
- Тесты: [tests](tests)
- Конфиг postman: [конфиг postman](postman/-api-v1.postman_collection.json)
- Dockerfile: [Dockerfile](Dockerfile)
- docker-compose.yml: [docker-compose.yml](docker-compose.yml)

---

## 8. Запуск

```
git clone https://github.com/crewch/software-engineering
cd lab2
docker-compose up
```

## Сборка из vscode в devcontainer

1. Скачать репозиторий и открыть его в vscode из директории lab2:

```
git clone https://github.com/crewch/software-engineering
cd lab2
code .
```

2. Скачать расширение ms-vscode-remote.remote-containers
3. Открыть проект в vscode и нажать 'Reopen in container'
4. Открыть терминал от пользователя user (можно в vscode Terminal -> New Terminal либо через docker exec и sudo su user)
5. Собрать проект с помощью:

```sh
make build-debug
```

6. Прописать симлинку, чтобы в vscode не было ошибок

```sh
ln -sf build-debug/compile_commands.json .
```

## Запуск из devcontainer

```sh
make start-debug
```

## Тесты из devcontainer

```sh
make test-debug
```
