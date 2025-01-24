 Установка через vcpkg (рекомендуется)
vcpkg — это менеджер пакетов от Microsoft, который упрощает установку библиотек.

Шаги:
Установите vcpkg:

Скачайте vcpkg с официального репозитория.

Распакуйте архив в удобную папку, например: C:\vcpkg.

Откройте командную строку и выполните:

bash
Copy
cd C:\vcpkg
.\bootstrap-vcpkg.bat
Установите Boost:

Для установки всех библиотек Boost:

bash
Copy
.\vcpkg install boost:x64-windows  # Для 64-битной версии
.\vcpkg install boost:x86-windows  # Для 32-битной
Если нужны только определенные библиотеки (например, Boost.Filesystem):

bash
Copy
.\vcpkg install boost-filesystem:x64-windows
Интеграция с Visual Studio:

Выполните команду для автоматической интеграции:

bash
Copy
.\vcpkg integrate install
Теперь Boost будет доступен в ваших проектах без дополнительных настроек.


boost-locale:x64-windows






https://apps.microsoft.com/detail/9n0dx20hk701?hl=ru-RU&gl=RU


. Настройка профиля в Windows Terminal
Установите Windows Terminal, если ещё не сделали этого.

Откройте настройки Windows Terminal (Ctrl + ,).

В разделе profiles → list добавьте новый профиль:

json
Copy
{
    "name": "MyApp (Release)",
    "commandline": "cmd.exe /k \"cd /d C:\\Projects\\MyApp\\bin\\Release && MyApp.exe\"",
    "hidden": false,
    "startingDirectory": "C:\\Projects\\MyApp\\bin\\Release"
}
Замените пути на актуальные для вашего проекта.

2. Запуск через Windows Terminal из Visual Studio
В Visual Studio перейдите в Tools → External Tools → Add.

Заполните поля:

Title: Запуск в Windows Terminal

Command: C:\Users\<user>\AppData\Local\Microsoft\WindowsApps\wt.exe

Arguments: -p "MyApp (Release" (название профиля из шага 1)

Initial directory: $(TargetDir)

Нажмите OK.

Теперь вы можете запускать приложение через меню Tools → External Tools → Запуск в Windows Terminal.