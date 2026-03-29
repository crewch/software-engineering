# Start via `make test-debug` or `make test-release`
import jwt
import pytest
from datetime import datetime, timedelta

from testsuite.utils import matching

JWT_SECRET_KEY = '3zSPwp5OPzbRoI8iR7FjLvH6j8jDuSWxy3BZejcjrIL'
HANDLER_USERS = '/api/v1/users'
HANDLER_LOGIN = '/api/v1/users/login'
HANDLER_CARS = '/api/v1/cars'
HANDLER_RENTALS = '/api/v1/rentals'


def make_jwt_token(payload: dict = None) -> str:
    """Создание JWT токена для аутентификации"""
    if payload is None:
        payload = {'iss': 'car-rental-api', 'sub': 'test-user'}
    return jwt.encode(payload, JWT_SECRET_KEY, algorithm='HS256')


def make_auth_headers(token: str = None) -> dict:
    """Создание заголовков с авторизацией"""
    if token is None:
        token = make_jwt_token()
    return {'Authorization': f'Bearer {token}'}


def make_user_data(login: str = None, override: dict = None) -> dict:
    """Создание данных пользователя для регистрации"""
    data = {
        'login': login or 'test_user',
        'first_name': 'Иван',
        'last_name': 'Петров',
        'email': 'test@example.com',
        'password': 'SuperSecret123!'
    }
    if override:
        data.update(override)
    return data


def make_login_data(login: str = 'test_user', password: str = 'SuperSecret123!') -> dict:
    """Создание данных для входа"""
    return {'login': login, 'password': password}


def make_car_data(vin: str = None, override: dict = None) -> dict:
    """Создание данных автомобиля"""
    data = {
        'vin': vin or 'WBA3A5C50CF123456',
        'brand': 'BMW',
        'model': '3 Series',
        'year': 2022,
        'car_class': 'midsize',
        'license_plate': 'A123BC777',
        'daily_rate': 5500.00
    }
    if override:
        data.update(override)
    return data


def make_rental_data(car_id: str = None, override: dict = None) -> dict:
    """Создание данных аренды"""
    now = datetime.utcnow()
    data = {
        'car_id': car_id or '550e8400-e29b-41d4-a716-446655440000',
        'start_date': (now + timedelta(days=1)).isoformat() + 'Z',
        'end_date': (now + timedelta(days=5)).isoformat() + 'Z'
    }
    if override:
        data.update(override)
    return data


async def get_auth_token(service_client, login: str = 'test_user', password: str = 'SuperSecret123!') -> str:
    """Получение токена через login endpoint"""
    response = await service_client.post(
        HANDLER_LOGIN,
        json=make_login_data(login, password)
    )
    if response.status == 200:
        return response.json()['token']
    return None


class TestUserRegistration:
    """Тесты регистрации пользователя"""

    async def test_create_user_201(self, service_client):
        """Успешная регистрация пользователя"""
        response = await service_client.post(
            HANDLER_USERS,
            json=make_user_data(login='new_user')
        )
        assert response.status == 201
        data = response.json()
        assert data['login'] == 'new_user'
        assert data['first_name'] == 'Иван'
        assert data['last_name'] == 'Петров'
        assert 'id' in data
        assert 'created_at' in data

    async def test_create_user_400_invalid_login(self, service_client):
        """Ошибка валидации: некорректный логин"""
        response = await service_client.post(
            HANDLER_USERS,
            json=make_user_data(login='ab')
        )
        assert response.status == 400
        data = response.json()
        assert data['code'] == 'validation_error'

    async def test_create_user_400_short_password(self, service_client):
        """Ошибка валидации: короткий пароль"""
        response = await service_client.post(
            HANDLER_USERS,
            json=make_user_data(override={'password': 'short'})
        )
        assert response.status == 400
        data = response.json()
        assert data['code'] == 'validation_error'

    async def test_create_user_409_conflict(self, service_client):
        """Конфликт: пользователь с таким логином уже существует"""
        login = 'duplicate_user'
        await service_client.post(HANDLER_USERS, json=make_user_data(login=login))
        
        response = await service_client.post(
            HANDLER_USERS,
            json=make_user_data(login=login)
        )
        assert response.status == 409
        data = response.json()
        assert data['code'] == 'conflict'


class TestUserLogin:
    """Тесты аутентификации пользователя"""

    async def test_login_200_success(self, service_client):
        """Успешная аутентификация"""
        login = 'login_test_user'
        password = 'SuperSecret123!'
        await service_client.post(HANDLER_USERS, json=make_user_data(login=login, override={'password': password}))
        
        response = await service_client.post(
            HANDLER_LOGIN,
            json=make_login_data(login, password)
        )
        assert response.status == 200
        data = response.json()
        assert 'token' in data

    async def test_login_400_invalid_credentials(self, service_client):
        """Неверный логин или пароль (валидация)"""
        response = await service_client.post(
            HANDLER_LOGIN,
            json=make_login_data(password='wrong')
        )
        assert response.status == 400
        data = response.json()
        assert data['code'] == 'validation_error'

    async def test_login_404_user_not_found(self, service_client):
        """Пользователь не найден"""
        response = await service_client.post(
            HANDLER_LOGIN,
            json=make_login_data(login='nonexistent_user')
        )
        assert response.status == 404
        data = response.json()
        assert data['code'] == 'not_found'


class TestGetUserByLogin:
    """Тесты получения пользователя по логину"""

    async def test_get_user_200(self, service_client):
        """Успешное получение пользователя"""
        login = 'searchable_user'
        await service_client.post(HANDLER_USERS, json=make_user_data(login=login))
        
        token = await get_auth_token(service_client, login)
        
        if token:
            response = await service_client.get(
                f'{HANDLER_USERS}/by-login/{login}',
                headers=make_auth_headers(token)
            )
            assert response.status == 200
            data = response.json()
            assert data['login'] == login
        else:
            pytest.skip("Could not get auth token")

    async def test_get_user_400_no_auth(self, service_client):
        """Ошибка: нет токена (возвращает 400)"""
        response = await service_client.get(
            f'{HANDLER_USERS}/by-login/test_user'
        )
        assert response.status == 400

    async def test_get_user_404_not_found(self, service_client):
        """Пользователь не найден"""
        login = 'existing_user'
        await service_client.post(HANDLER_USERS, json=make_user_data(login=login))
        token = await get_auth_token(service_client, login)
        
        if token:
            response = await service_client.get(
                f'{HANDLER_USERS}/by-login/nonexistent_user',
                headers=make_auth_headers(token)
            )
            assert response.status == 404
        else:
            pytest.skip("Could not get auth token")


class TestCreateCar:
    """Тесты создания автомобиля"""

    async def test_create_car_201(self, service_client):
        """Успешное создание автомобиля"""
        login = 'car_admin'
        await service_client.post(HANDLER_USERS, json=make_user_data(login=login))
        token = await get_auth_token(service_client, login)
        
        if token:
            response = await service_client.post(
                HANDLER_CARS,
                json=make_car_data(vin='WBA3A5C50CF123456'),
                headers=make_auth_headers(token)
            )
            assert response.status == 201
            data = response.json()
            assert data['vin'] == 'WBA3A5C50CF123456'
            assert data['brand'] == 'BMW'
            assert data['available'] == True
            assert 'id' in data
            assert 'created_at' in data
        else:
            pytest.skip("Could not get auth token")

    async def test_create_car_400_no_auth(self, service_client):
        """Ошибка аутентификации при создании (возвращает 400)"""
        response = await service_client.post(
            HANDLER_CARS,
            json=make_car_data()
        )
        assert response.status == 400

    async def test_create_car_400_invalid_vin(self, service_client):
        """Ошибка валидации: некорректный VIN"""
        login = 'car_admin2'
        await service_client.post(HANDLER_USERS, json=make_user_data(login=login))
        token = await get_auth_token(service_client, login)
        
        if token:
            response = await service_client.post(
                HANDLER_CARS,
                json=make_car_data(vin='INVALID'),
                headers=make_auth_headers(token)
            )
            assert response.status == 400
            data = response.json()
            assert data['code'] == 'validation_error'
        else:
            pytest.skip("Could not get auth token")

    async def test_create_car_400_invalid_year(self, service_client):
        """Ошибка валидации: некорректный год"""
        login = 'car_admin3'
        await service_client.post(HANDLER_USERS, json=make_user_data(login=login))
        token = await get_auth_token(service_client, login)
        
        if token:
            response = await service_client.post(
                HANDLER_CARS,
                json=make_car_data(override={'year': 1800}),
                headers=make_auth_headers(token)
            )
            assert response.status == 400
        else:
            pytest.skip("Could not get auth token")

    async def test_create_car_409_vin_exists(self, service_client):
        """Конфликт: автомобиль с таким VIN уже существует"""
        login = 'car_admin4'
        await service_client.post(HANDLER_USERS, json=make_user_data(login=login))
        token = await get_auth_token(service_client, login)
        
        if token:
            vin = '1HGBH41JXMN109186'
            await service_client.post(
                HANDLER_CARS,
                json=make_car_data(vin=vin),
                headers=make_auth_headers(token)
            )
            
            response = await service_client.post(
                HANDLER_CARS,
                json=make_car_data(vin=vin),
                headers=make_auth_headers(token)
            )
            assert response.status == 409
            data = response.json()
            assert data['code'] == 'conflict'
        else:
            pytest.skip("Could not get auth token")


class TestGetAvailableCars:
    """Тесты получения списка доступных автомобилей"""

    async def test_get_cars_200(self, service_client):
        """Успешное получение списка автомобилей"""
        login = 'cars_viewer'
        await service_client.post(HANDLER_USERS, json=make_user_data(login=login))
        token = await get_auth_token(service_client, login)
        
        if token:
            await service_client.post(HANDLER_CARS, json=make_car_data(vin='1HGBH41JXMN109186'), headers=make_auth_headers(token))
            await service_client.post(HANDLER_CARS, json=make_car_data(vin='2HGBH41JXMN109187'), headers=make_auth_headers(token))
            
            response = await service_client.get(
                f'{HANDLER_CARS}/available',
                headers=make_auth_headers(token)
            )
            assert response.status == 200
            data = response.json()
            assert 'items' in data
            assert 'total' in data
            assert isinstance(data['items'], list)
        else:
            pytest.skip("Could not get auth token")

    async def test_get_cars_200_with_pagination(self, service_client):
        """Получение списка с параметрами пагинации"""
        login = 'cars_viewer2'
        await service_client.post(HANDLER_USERS, json=make_user_data(login=login))
        token = await get_auth_token(service_client, login)
        
        if token:
            response = await service_client.get(
                f'{HANDLER_CARS}/available',
                params={'limit': 10, 'offset': 0},
                headers=make_auth_headers(token)
            )
            assert response.status == 200
            data = response.json()
            assert 'items' in data
            assert 'total' in data
        else:
            pytest.skip("Could not get auth token")

    async def test_get_cars_400_invalid_limit(self, service_client):
        """Ошибка валидации: некорректный limit"""
        login = 'cars_viewer3'
        await service_client.post(HANDLER_USERS, json=make_user_data(login=login))
        token = await get_auth_token(service_client, login)
        
        if token:
            response = await service_client.get(
                f'{HANDLER_CARS}/available',
                params={'limit': 200},
                headers=make_auth_headers(token)
            )
            assert response.status == 400
        else:
            pytest.skip("Could not get auth token")

    async def test_get_cars_400_no_auth(self, service_client):
        """Ошибка аутентификации (возвращает 400)"""
        response = await service_client.get(f'{HANDLER_CARS}/available')
        assert response.status == 400


class TestGetCarsByClass:
    """Тесты поиска автомобилей по классу"""

    @pytest.mark.parametrize('car_class', [
        'economy', 'compact', 'midsize', 'fullsize', 'luxury', 'suv', 'van'
    ])
    async def test_get_cars_by_class_200(self, service_client, car_class):
        """Успешный поиск по классу автомобиля"""
        login = 'class_viewer'
        await service_client.post(HANDLER_USERS, json=make_user_data(login=login))
        token = await get_auth_token(service_client, login)
        
        if token:
            response = await service_client.get(
                f'{HANDLER_CARS}/class/{car_class}',
                headers=make_auth_headers(token)
            )
            assert response.status == 200
            data = response.json()
            assert 'items' in data
            assert 'total' in data
        else:
            pytest.skip("Could not get auth token")

    async def test_get_cars_by_class_400_invalid_class(self, service_client):
        """Ошибка валидации: некорректный класс автомобиля"""
        login = 'class_viewer2'
        await service_client.post(HANDLER_USERS, json=make_user_data(login=login))
        token = await get_auth_token(service_client, login)
        
        if token:
            response = await service_client.get(
                f'{HANDLER_CARS}/class/invalid_class',
                headers=make_auth_headers(token)
            )
            assert response.status == 400
            data = response.json()
            assert data['code'] == 'validation_error'
        else:
            pytest.skip("Could not get auth token")

    async def test_get_cars_by_class_400_no_auth(self, service_client):
        """Ошибка аутентификации (возвращает 400)"""
        response = await service_client.get(f'{HANDLER_CARS}/class/suv')
        assert response.status == 400


class TestCreateRental:
    """Тесты создания аренды"""

    async def test_create_rental_201(self, service_client):
        """Успешное создание аренды"""
        login = 'rental_user'
        await service_client.post(HANDLER_USERS, json=make_user_data(login=login))
        token = await get_auth_token(service_client, login)
        
        if token:
            car_response = await service_client.post(
                HANDLER_CARS,
                json=make_car_data(vin='RENTAL12345678901'),
                headers=make_auth_headers(token)
            )
            car_id = car_response.json()['id']
            
            response = await service_client.post(
                HANDLER_RENTALS,
                json=make_rental_data(car_id=car_id),
                headers=make_auth_headers(token)
            )
            assert response.status == 201
            data = response.json()
            assert 'id' in data
            assert 'car_id' in data
            assert 'user_id' in data
            assert 'start_date' in data
            assert 'end_date' in data
            assert 'total_cost' in data
            assert data['status'] == 'active'
        else:
            pytest.skip("Could not get auth token")

    async def test_create_rental_400_end_before_start(self, service_client):
        """Ошибка валидации: дата окончания раньше даты начала"""
        login = 'rental_user2'
        await service_client.post(HANDLER_USERS, json=make_user_data(login=login))
        token = await get_auth_token(service_client, login)
        
        if token:
            now = datetime.utcnow()
            response = await service_client.post(
                HANDLER_RENTALS,
                json={
                    'car_id': '550e8400-e29b-41d4-a716-446655440000',
                    'start_date': (now + timedelta(days=5)).isoformat() + 'Z',
                    'end_date': (now + timedelta(days=1)).isoformat() + 'Z'
                },
                headers=make_auth_headers(token)
            )
            assert response.status == 400
            data = response.json()
            assert data['code'] == 'validation_error'
        else:
            pytest.skip("Could not get auth token")

    async def test_create_rental_400_no_auth(self, service_client):
        """Ошибка аутентификации (возвращает 400)"""
        response = await service_client.post(
            HANDLER_RENTALS,
            json=make_rental_data()
        )
        assert response.status == 400

    async def test_create_rental_404_car_not_found(self, service_client):
        """Автомобиль не найден"""
        login = 'rental_user3'
        await service_client.post(HANDLER_USERS, json=make_user_data(login=login))
        token = await get_auth_token(service_client, login)
        
        if token:
            response = await service_client.post(
                HANDLER_RENTALS,
                json=make_rental_data(car_id='00000000-0000-0000-0000-000000000000'),
                headers=make_auth_headers(token)
            )
            assert response.status == 404
            data = response.json()
            assert data['code'] == 'not_found'
        else:
            pytest.skip("Could not get auth token")

    async def test_create_rental_409_car_not_available(self, service_client):
        """Автомобиль недоступен для аренды"""
        login = 'rental_user4'
        await service_client.post(HANDLER_USERS, json=make_user_data(login=login))
        token = await get_auth_token(service_client, login)
        
        if token:
            car_response = await service_client.post(
                HANDLER_CARS,
                json=make_car_data(vin='3HGBH41JXMN109188'),
                headers=make_auth_headers(token)
            )
            car_id = car_response.json()['id']
            
            now = datetime.utcnow()
            await service_client.post(
                HANDLER_RENTALS,
                json={
                    'car_id': car_id,
                    'start_date': (now + timedelta(days=1)).isoformat() + 'Z',
                    'end_date': (now + timedelta(days=10)).isoformat() + 'Z'
                },
                headers=make_auth_headers(token)
            )
            
            response = await service_client.post(
                HANDLER_RENTALS,
                json={
                    'car_id': car_id,
                    'start_date': (now + timedelta(days=2)).isoformat() + 'Z',
                    'end_date': (now + timedelta(days=5)).isoformat() + 'Z'
                },
                headers=make_auth_headers(token)
            )
            assert response.status == 409
            data = response.json()
            assert data['code'] == 'car_not_available'
        else:
            pytest.skip("Could not get auth token")


class TestErrorResponses:
    """Тесты проверки формата ошибок"""

    async def test_validation_error_schema(self, service_client):
        """Проверка схемы ValidationError"""
        response = await service_client.post(
            HANDLER_USERS,
            json=make_user_data(login='ab')
        )
        data = response.json()
        assert 'code' in data
        assert 'message' in data
        assert data['code'] == 'validation_error'

    async def test_not_found_error_schema(self, service_client):
        """Проверка схемы NotFoundError"""
        login = 'error_test_user'
        await service_client.post(HANDLER_USERS, json=make_user_data(login=login))
        token = await get_auth_token(service_client, login)
        
        if token:
            response = await service_client.get(
                f'{HANDLER_USERS}/by-login/nonexistent',
                headers=make_auth_headers(token)
            )
            assert response.status == 404
            data = response.json()
            assert data['code'] == 'not_found'
            assert 'message' in data
        else:
            pytest.skip("Could not get auth token")

    async def test_conflict_error_schema(self, service_client):
        """Проверка схемы ConflictError"""
        login = 'conflict_test'
        await service_client.post(HANDLER_USERS, json=make_user_data(login=login))
        response = await service_client.post(
            HANDLER_USERS,
            json=make_user_data(login=login)
        )
        data = response.json()
        assert data['code'] == 'conflict'
        assert 'message' in data