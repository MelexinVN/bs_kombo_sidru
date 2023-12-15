<img align="right" width=150 src="https://github.com/MelexinVN/bs_kombo_sidru/blob/main/media/photo.jpg" />

# СветоИзлучающее Декоративное Радиоуправляемое Устройство (СИДРУ)
## Проект ["КомБО"](https://github.com/MelexinVN/bs_kombo) (Открытые системы беспроводной коммуникации)

<img align="right" width=120 src="https://github.com/MelexinVN/bs_kombo_sidru/blob/main/media/logo_red.png" />

В рамках проекта "КомБО" разработано декоративное устройство на адресных светодиодах, управляемое по радиоканалу.

Устройство состоит из двух частей - декоративная часть и пульт управления.

Обе части содержат схему управления на микроконтроллере stm32f030f4p6, радиомодуль nrf24l01+, и схему питания.
В декоративной части микроконтроллер управляет адресными светодиодами ws2812. 
В данном случае светодиодны расположены на модуле в виде кольца (могут быть отдельными модулями, в виде светодиодной ленты и т.п.)

В пульте управления микроконтроллер в зависимости от состояния чувствительного элемента отправляет различные команды в ответ на запросы декоративного устройства.

Здесь в качестве чувствительных элементов используются датчики касания TTP223 (могут быть использованы кнопки, реле и т.п.)

As part of the "КомБО" project, a decorative device based on addressable LEDs, controlled via a radio channel, has been developed.

The device consists of two parts - a decorative part and a control panel.

Both parts contain a control circuit on the stm32f030f4p6 microcontroller, an nrf24l01+ radio module, and a power supply circuit.
In the decorative part, the microcontroller controls the ws2812 addressable LEDs.
In this case, the LEDs are located on the module in the form of a ring (can be separate modules, in the form of an LED strip, etc.)

In the control panel, the microcontroller, depending on the state of the sensitive element, sends various commands in response to requests from the decorative device.

Here, TTP223 touch sensors are used as sensitive elements (buttons, relays, etc. can be used)


### Репозиторий содержит:
- ***Исполняемые файлы и исходники ПО декоративной части и пульта управления***. Программы разработаны в Keil µVision V5.38.0.0 (MDK-Lite).
- ***Гербер-файлы и исходники проектов используемых печатных плат***. Платы разработаны в KiCad 7.0.1, дополнительно в папках проекта можно найти архив 3д-моделей компонентов, а также интерактивный список компонентов (BOM).
- ***STL-файлы и исходники 3д-моделей используемых 3д-печатных деталей и корпусов***. Модели разработаны в OpenSCAD version 2021.01.

### The repository contains:
- ***Executable files and software sources for the decorative part and the control panel***. The programs were developed in Keil µVision V5.38.0.0 (MDK-Lite).
- ***Gerber files and source codes for designs of printed circuit boards used***. The boards were developed in KiCad 7.0.1; in addition, in the project folders you can find an archive of 3D models of components, as well as an interactive list of components (BOM).
- ***STL files and sources of 3D models of used 3D printed parts and cases***. The models were developed in OpenSCAD version 2021.01.

## Схема соединений пульта управления. Control panel connection diagram

<img align="center" width=500 src="https://github.com/MelexinVN/bs_kombo_sidru/blob/main/media/remote_schematic.png" />

## Схема соединений декоративной части. Connection diagram of the decorative part

<img align="center" width=500 src="https://github.com/MelexinVN/bs_kombo_sidru/blob/main/media/sidru_schematic.png" />


## Наши ресурсы:

### Основной репозиторий: [bs_kombo](https://github.com/MelexinVN/bs_kombo)

### Cайт проекта: [bs-kombo.tilda.ws](http://bs-kombo.tilda.ws/)

### Электронная почта: 		    [bskombo@yandex.ru](bskombo@yandex.ru)

### Социальные сети:

- паблик ВКонтакте: 			  [vk.com/bs_kombo](https://vk.com/bs_kombo)
- Telegram-канал: 	        [t.me/bskombo](https://t.me/bskombo)
- Telegram-чат: 	          [t.me/bs_kombo_chat](https://t.me/bs_kombo_chat)
- канал Яндекс Дзен: 	      [dzen.ru/bs_kombo](https://dzen.ru/bs_kombo)


<img align="left" width=150 src="https://github.com/MelexinVN/bs_kombo_sidru/blob/main/media/%D0%9C%D0%9D%D0%A5%D0%A1.png" />

### Авторы всегда рады отзывам и любым конструктивным предложениям по улучшению проекта! 
### The authors are always happy to receive feedback and any constructive suggestions for improving the project!

## Лицензия
Проект распространяется под лицензией Creative Commons Attribution-ShareAlike 4.0 [CC BY-SA 4.0](https://creativecommons.org/licenses/by-sa/4.0/), 
а программное обеспечение - под лицензией GNU General Public License v3.0 [GPL-3.0 license](https://github.com/MelexinVN/bs_kombo/blob/main/LICENSE)

## License
This project is licensed under the Creative Commons 4.0 license with Attribution-ShareAlike [CC BY-SA 4.0](https://creativecommons.org/licenses/by-sa/4.0/), 
and the software is licensed under the GNU General Public License v3.0 [GPL-3.0 license](https://github.com/MelexinVN/bs_kombo/blob/main/LICENSE)
