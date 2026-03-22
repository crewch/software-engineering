# Start via `make test-debug` or `make test-release`
import jwt
import pytest

from testsuite.utils import matching

JWT_SECRET_KEY = '3zSPwp5OPzbRoI8iR7FjLvH6j8jDuSWxy3BZejcjrIL'
OTHER_JWT_SECRET_KEY = ''

HANDLER_URL = '/hello'


def make_headers(payload={'iss': 'sample'}):
    token = jwt.encode(payload, JWT_SECRET_KEY, algorithm='HS256')
    return {'Authorization': f'Bearer {token}'}


def make_name(name):
    return {'name': name} if name is not None else {}


@pytest.mark.parametrize('name_param, json_name, expected_name',
                         [pytest.param(None, None, 'noname', id='Default name with no args'),
                          pytest.param('Userver', None, 'Userver',
                                       id='Name from query parameters'),
                             pytest.param(None, 'Userver',
                                          'Userver', id='Name from body'),
                             pytest.param('Userver', None, 'Userver', id='Query name overwrites default')]
                         )
async def test_200(service_client, name_param, json_name, expected_name):
    response = await service_client.post(HANDLER_URL, params=make_name(name_param), headers=make_headers(), json=make_name(json_name))
    assert response.status == 200
    assert response.json() == {
        'text': f'Hello to {expected_name} from Userver',
        'current-time': matching.datetime_string
    }


@pytest.mark.parametrize('payload', [pytest.param({}, id='Empty payload'), pytest.param({'iss': 'other'}, id='Other issuer')])
async def test_401(service_client, payload):
    response = await service_client.post(HANDLER_URL, params={'name': 'Userver'}, headers=make_headers(payload), json={})
    assert response.status == 401
