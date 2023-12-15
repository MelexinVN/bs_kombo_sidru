/***************************************************************
sidru_plate.scad
Программа генерации 3д модели основания для компактного варианта
СветоИзлучающего Декоративного Радиоуправляемого Устройства
(СИДРУ) проекта КомБО (Открытые системы беспроводной коммуникации)

Автор: MelexinVN (Мелехин В.Н.)
***************************************************************/

$fn = 100;          //количество полигонов

height = 2;         //толщина основания
ext_diameter = 68;  //диаметр основания
int_diameter = 1;   //диаметр внутренних отверстий

difference()    //разность
{
    cylinder(h = height, d = ext_diameter, center = true); 
    //из цилиндра с заданными параметрами
    //вычитаем объединение фигур
    union()
    {
    //Два смещенный на определенное расстоение параллелипипида, задающие вырезы для вывода проводов (для генерации задней крышки без этих вырезов закомментировать их)
        translate([0,-ext_diameter/2 + 2,0])
            cube([10,2,height+1], center = true);
        translate([0,-12.7,0])
            cube([20,3,height+1], center = true);
     //Ряд смещенных на определенное расстояние цилиндров, задающих отверстия для крепежа печатных плат
        translate([12.7,12.7,0])
            cylinder(h = height+1, d = int_diameter, center = true);
        translate([12.7,-12.7,0])
            cylinder(h = height+1, d = int_diameter, center = true);
        translate([-12.7,12.7,0])
            cylinder(h = height+1, d = int_diameter, center = true);
        translate([-12.7,-12.7,0])
            cylinder(h = height+1, d = int_diameter, center = true);
        translate([26,18,0])
            cylinder(h = height+1, d = int_diameter, center = true);
        translate([26,-18,0])
            cylinder(h = height+1, d = int_diameter, center = true);
        translate([-26,18,0])
            cylinder(h = height+1, d = int_diameter, center = true);
        translate([-26,-18,0])
            cylinder(h = height+1, d = int_diameter, center = true);
    }
}