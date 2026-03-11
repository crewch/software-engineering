workspace "Car Rental System" "Система аренды автомобилей" {
    model {
        guest = person "Гость" "Незарегистрированный посетитель. Выполняет поиск автомобилей, просматривает наличие и цены." "Person"
        registeredCustomer = person "Зарегистрированный клиент" "Клиент с аккаунтом. Создаёт бронирования, выбирает доп. услуги, управляет заказами." "Person"
        corporateClient = person "Корпоративный клиент" "Пользователь корпоративного аккаунта. Специальные тарифы и отчёты по расходам." "Person"
        hertzStaff = person "Администратор" "Внутренний персонал: менеджеры флота, бронирований, поддержки и финансов." "Person"

        paymentGateways = softwareSystem "Платежные шлюзы и процессоры" "Приём оплаты, предоплаты и возвратов." "External"
        emailServices = softwareSystem "Email-сервисы" "Отправка подтверждений, ваучеров и маркетинга." "External"
        pushServices = softwareSystem "Сервисы Push-уведомлений" "Мобильные оповещения и коды доступа к авто." "External"
        mapsServices = softwareSystem "Сервисы геолокации и карт" "Отображение пунктов проката." "External"
        verificationServices = softwareSystem "Системы верификации документов" "Проверка прав, паспортов и кредитной истории." "External"

        hertz = softwareSystem "Car Rental System" {
            gateway = container "API Gateway" "Проксирование запросов, балансировка нагрузки, аутентификация и SSL" "Nginx" "Container"
            
            bookingService = container "Booking Service" "Управление бронированиями, расчёт стоимости и one-way аренды" "c++ / userver" "Container"
            fleetService = container "Fleet Management Service" "Управление парком авто, наличием и локациями" "c++ / userver" "Container"
            customerService = container "Customer Service" "Управление аккаунтами клиентов и корпоративными профилями" "c++ / userver" "Container"
            paymentService = container "Payment Service" "Обработка платежей и интеграция со шлюзами" "c++ / userver" "Container"
            
            notificationService = container "Notification Service" "Генерация уведомлений и постановка задач в очередь RabbitMQ" "c++ / userver" "Container"
            emailService = container "Email Service" "Обработка очереди email-уведомлений и отправка" "c++ / userver" "Container"
            pushService = container "Push Service" "Обработка очереди push-уведомлений и отправка" "c++ / userver" "Container"
            
            adminService = container "Admin Service" "Административные операции для сотрудников" "c++ / userver" "Container"
            locationService = container "Location Service" "Интеграция с картами и геолокацией пунктов проката" "c++ / userver" "Container"
            verificationService = container "Verification Service" "Внутренняя верификация документов клиентов" "c++ / userver" "Container"
            
            rentalDb = container "Rental Database" "Реляционное хранилище бронирований, заказов и флота" "PostgreSQL" "Database"
            userDb = container "User Database" "Реляционное хранилище аккаунтов клиентов и сотрудников" "PostgreSQL" "Database"
            cache = container "Cache" "Кэширование наличия авто, цен и сессий" "Redis" "Database"
            
            messageBroker = container "Message Broker" "Асинхронная доставка уведомлений и заданий (RabbitMQ)" "RabbitMQ" "Broker"
        }

        guest -> gateway "Поиск и просмотр автомобилей" "HTTPS/JSON"
        registeredCustomer -> gateway "Бронирование, оплата и управление заказами" "HTTPS/JSON"
        corporateClient -> gateway "Корпоративное бронирование и отчёты" "HTTPS/JSON"
        hertzStaff -> gateway "Управление флотом, бронированиями и поддержкой" "HTTPS/JSON"

        paymentGateways -> gateway "Подтверждение платежа" "HTTP/JSON"
        gateway -> paymentGateways "Инициация платежа" "HTTPS/JSON"

        gateway -> customerService "Авторизация и профиль клиента" "HTTP/JSON"
        gateway -> bookingService "Создание/изменение бронирования" "HTTP/JSON"
        gateway -> fleetService "Проверка наличия авто" "HTTP/JSON"
        gateway -> adminService "Административные запросы" "HTTP/JSON"
        gateway -> locationService "Данные карт и локаций" "HTTP/JSON"
        gateway -> verificationService "Запрос верификации документов" "HTTP/JSON"

        customerService -> userDb "Сохранение/чтение аккаунтов" "TCP/SQL"
        bookingService -> rentalDb "Сохранение бронирований" "TCP/SQL"
        bookingService -> fleetService "Проверка и резервирование авто" "HTTP/JSON"
        fleetService -> rentalDb "Обновление статуса флота" "TCP/SQL"
        bookingService -> paymentService "Инициация оплаты" "HTTP/JSON"
        paymentService -> paymentGateways "Обработка платежа" "HTTPS/JSON"
        paymentService -> notificationService "Уведомление об оплате" "HTTP/JSON"
        bookingService -> notificationService "Подтверждение бронирования" "HTTP/JSON"

        notificationService -> messageBroker "Отправка уведомлений в очередь" "AMQP"
        messageBroker -> emailService "Доставка email-уведомлений из очереди" "AMQP"
        messageBroker -> pushService "Доставка push-уведомлений из очереди" "AMQP"
        emailService -> emailServices "Отправка email" "SMTP"
        pushService -> pushServices "Отправка push-уведомлений" "API"

        locationService -> mapsServices "Запрос геоданных" "REST API"
        verificationService -> verificationServices "Верификация документов" "REST API / HTTPS"
        bookingService -> cache "Кэширование цен и наличия" "TCP/JSON"
        fleetService -> cache "Кэширование инвентаря" "TCP/JSON"
    }

    views {
        systemContext hertz "ContextView" {
            include *
            autolayout lr
        }

        container hertz "ContainerView" {
            include *
            autolayout lr
        }

        dynamic hertz "BookingCarUsecase" {
            registeredCustomer -> gateway "Клиент отправляет запрос на бронирование автомобиля"
            gateway -> customerService "Проверка авторизации и статуса клиента"
            customerService -> userDb "Проверка аккаунта и прав"
            gateway -> bookingService "Создание бронирования"
            gateway -> locationService "Запрос на проверку геоданных"
            locationService -> mapsServices "Проверка геоданных через внешний шлюз"
            bookingService -> fleetService "Проверка наличия авто в выбранной локации"
            fleetService -> rentalDb "Обновление статуса автомобиля"
            bookingService -> rentalDb "Сохранение записи о бронировании"
            bookingService -> paymentService "Инициация оплаты"
            paymentService -> paymentGateways "Обработка платежа через внешний шлюз"
            paymentService -> notificationService "Уведомление о успешной оплате"
            bookingService -> notificationService "Подтверждение бронирования"
            notificationService -> messageBroker "Постановка уведомления в очередь RabbitMQ"
            messageBroker -> emailService "Доставка задачи email-уведомления"
            messageBroker -> pushService "Доставка задачи push-уведомления"
            emailService -> emailServices "Отправка ваучера и подтверждения на email"
            pushService -> pushServices "Отправка push-уведомления клиенту"
        }

        styles {
            element "Database" {
                shape Cylinder
                background #1168bd
                color #ffffff
            }
            element "Broker" {
                shape Pipe
                background #d35400
                color #ffffff
            }
            element "Container" {
                background #438dd5
                color #ffffff
            }
            element "Person" {
                shape Person
                background #08427b
                color #ffffff
            }
        }
    }
}