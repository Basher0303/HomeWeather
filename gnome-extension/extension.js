/* extension.js
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

/* exported init */

const GETTEXT_DOMAIN = "my-indicator-extension";

const { GObject, St, Gio, Soup, GLib, Clutter } = imports.gi;

const ExtensionUtils = imports.misc.extensionUtils;
const Main = imports.ui.main;
const PanelMenu = imports.ui.panelMenu;
const PopupMenu = imports.ui.popupMenu;

const _ = ExtensionUtils.gettext;

const Indicator = GObject.registerClass(
    class Indicator extends PanelMenu.Button {
        _init() {
            super._init(0.5, _("My Shiny Indicator"));

            let Me = ExtensionUtils.getCurrentExtension();

            let panelBox = new St.BoxLayout({ style_class: "indicator-box" });
            let homeIcon = new St.Icon({
                gicon: Gio.FileIcon.new(Gio.File.new_for_path(Me.path + "/src/icons/home-symbolic.svg")),
                icon_size: 18,
                style_class: "indicator-home-icon",
            });
            panelBox.add_child(homeIcon);

            this._label = new St.Label({
                text: "*",
                y_align: Clutter.ActorAlign.CENTER,
            });
            panelBox.add_child(this._label);

            this.add_child(panelBox);

            // Добавляем dropdown с вёрсткой из 3 лейблов с иконками
            let section = new PopupMenu.PopupMenuSection();
            let box = new St.BoxLayout({ vertical: true });

            let tempRow = new St.BoxLayout({ style_class: "weather-row" });
            let tempIcon = new St.Icon({
                gicon: Gio.FileIcon.new(Gio.File.new_for_path(Me.path + "/src/icons/temperature.svg")),
                icon_size: 20,
                style_class: "weather-icon",
            });
            tempRow.add_child(tempIcon);
            tempRow.add_child(
                new St.Label({ text: "Температура", style_class: "weather-menu-label weather-key-label" }),
            );
            this._tempValueLabel = new St.Label({
                text: "--",
                style_class: "weather-menu-label weather-value-label",
                x_expand: true,
                x_align: Clutter.ActorAlign.END,
            });
            tempRow.add_child(this._tempValueLabel);
            box.add_child(tempRow);

            let humidRow = new St.BoxLayout({ style_class: "weather-row" });
            let humidIcon = new St.Icon({
                gicon: Gio.FileIcon.new(Gio.File.new_for_path(Me.path + "/src/icons/humidity.svg")),
                icon_size: 20,
                style_class: "weather-icon",
            });
            humidRow.add_child(humidIcon);
            humidRow.add_child(
                new St.Label({ text: "Влажность", style_class: "weather-menu-label weather-key-label" }),
            );
            this._humidValueLabel = new St.Label({
                text: "--",
                style_class: "weather-menu-label weather-value-label",
                x_expand: true,
                x_align: Clutter.ActorAlign.END,
            });
            humidRow.add_child(this._humidValueLabel);
            box.add_child(humidRow);

            let pressRow = new St.BoxLayout({ style_class: "weather-row" });
            let pressIcon = new St.Icon({
                gicon: Gio.FileIcon.new(Gio.File.new_for_path(Me.path + "/src/icons/pressure.svg")),
                icon_size: 20,
                style_class: "weather-icon",
            });
            pressRow.add_child(pressIcon);
            pressRow.add_child(new St.Label({ text: "Давление", style_class: "weather-menu-label weather-key-label" }));
            this._pressValueLabel = new St.Label({
                text: "--",
                style_class: "weather-menu-label weather-value-label",
                x_expand: true,
                x_align: Clutter.ActorAlign.END,
            });
            pressRow.add_child(this._pressValueLabel);
            box.add_child(pressRow);

            let co2Row = new St.BoxLayout({ style_class: "weather-row" });
            let co2Icon = new St.Icon({
                gicon: Gio.FileIcon.new(Gio.File.new_for_path(Me.path + "/src/icons/co2.svg")),
                icon_size: 20,
                style_class: "weather-icon",
            });
            co2Row.add_child(co2Icon);
            co2Row.add_child(new St.Label({ text: "CO2", style_class: "weather-menu-label weather-key-label" }));
            this._co2ValueLabel = new St.Label({
                text: "--",
                style_class: "weather-menu-label weather-value-label",
                x_expand: true,
                x_align: Clutter.ActorAlign.END,
            });
            co2Row.add_child(this._co2ValueLabel);
            box.add_child(co2Row);

            let separator = new PopupMenu.PopupSeparatorMenuItem();
            box.add_child(separator);

            this._updateTimeLabel = new St.Label({
                text: "Обновлено: --",
                style_class: "weather-menu-label-time",
                x_align: Clutter.ActorAlign.CENTER,
            });
            box.add_child(this._updateTimeLabel);

            section.actor.add_child(box);
            this.menu.addMenuItem(section);
        }
    },
);

class Extension {
    constructor(uuid) {
        this._uuid = uuid;
        this._lastCo2NotificationTime = 0;

        ExtensionUtils.initTranslations(GETTEXT_DOMAIN);
    }

    enable() {
        this._indicator = new Indicator();
        Main.panel.addToStatusArea(this._uuid, this._indicator, 1, "center");
        global.log("Привет, консоль!"); // выведет в стандартный журнал GNOME Shell

        const updateWeather = () => {
            let url = "http://api.alex-basher.ru/get";

            this._httpSession = new Soup.SessionAsync();
            const message = Soup.Message.new("GET", url);
            let indicator = this._indicator; // Сохраняем ссылку на индикатор

            this._httpSession.queue_message(message, (session, msg) => {
                if (msg.status_code !== 200) {
                    log(`HTTP Error: ${msg.status_code}`);
                    indicator._label.set_text("Error");
                    return;
                }

                try {
                    const json = JSON.parse(msg.response_body.data);
                    indicator._label.set_text(`${json.temperature}°C`);
                    indicator._tempValueLabel.set_text(`${json.temperature}°C`);
                    if (json.humidity !== undefined) {
                        indicator._humidValueLabel.set_text(`${json.humidity}%`);
                    }
                    if (json.pressure !== undefined) {
                        indicator._pressValueLabel.set_text(`${json.pressure} мм`);
                    }
                    if (json.co2 !== undefined) {
                        indicator._co2ValueLabel.set_text(`${json.co2} ppm`);
                        if (Number(json.co2) > 1000) {
                            const now = GLib.get_monotonic_time();
                            const interval = 5 * 60 * 1000000; // 5 минут в микросекундах
                            if (now - this._lastCo2NotificationTime >= interval) {
                                this._lastCo2NotificationTime = now;
                                Main.notify("CO2 превышен", `Уровень CO2 ${json.co2} ppm выше порога 1000 ppm`);
                            }
                        }
                    }

                    const now = new Date();
                    const hours = String(now.getHours()).padStart(2, "0");
                    const minutes = String(now.getMinutes()).padStart(2, "0");
                    const seconds = String(now.getSeconds()).padStart(2, "0");
                    indicator._updateTimeLabel.set_text(`Обновлено: ${hours}:${minutes}:${seconds}`);
                } catch (e) {
                    global.log(e);
                    indicator._label.set_text("Parse Error");
                }
            });
        };

        updateWeather();

        // Таймер обновления каждые 10 секунд
        this._timeout = GLib.timeout_add_seconds(GLib.PRIORITY_DEFAULT, 5, () => {
            updateWeather();
            return GLib.SOURCE_CONTINUE;
        });
    }

    disable() {
        this._indicator.destroy();
        this._indicator = null;
    }
}

function init(meta) {
    return new Extension(meta.uuid);
}
