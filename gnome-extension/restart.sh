#!/bin/bash

# Перезагрузить расширение GNOME Shell

# Сначала пытаемся использовать gnome-extensions
if command -v gnome-extensions &> /dev/null; then
    echo "Отключаем расширение..."
    gnome-extensions disable home-weather@basher.com 2>/dev/null
    sleep 1
    echo "Включаем расширение..."
    gnome-extensions enable home-weather@basher.com 2>/dev/null
    echo "Расширение перезагружено!"
else
    # Альтернативный метод через dbus
    dbus-send --print-reply --session \
        --dest=org.gnome.Shell \
        /org/gnome/Shell \
        org.gnome.Shell.Extensions.ReloadExtension \
        string:"home-weather@basher.com" 2>/dev/null || \
    dbus-send --print-reply --session \
        /org/gnome/Shell \
        org.gnome.Shell.Eval \
        string:'Main.extensionManager.reloadExtension("home-weather@basher.com")' 2>/dev/null || \
    echo "⚠️  Не удалось перезагрузить автоматически. Отключите и включите расширение в приложении Расширения."
fi
