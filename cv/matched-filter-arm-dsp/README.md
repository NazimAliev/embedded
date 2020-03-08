###Согласованный фильтр поиска шаблонов в изображениях: embedded-реализация на ARM и DSP

Согласованный фильтр используется для нахождения положения шаблона на изображении. Применяется для обработки видеопотока камеры дрона для быстродействующего поиска посадочных маркеров.

Высоконагруженный алгоритм фильтра включает двумерное быстрое преобразование Фурье (FFT2), выполнение которого производися в подсистеме DSP (каталоги server_dsp, codec_dsp). Основное приложение, выполняющее интерфейсные функции, разработано для подсистемы ARM (main_app).

Структура каталогов соответствует разработке для процессоров архитектуры OMAP Texas Instruments, где для обмена между ARM и DSP используется библиотека Codec Engine.

[Принцип работы](http://www.nazim.ru/2269)