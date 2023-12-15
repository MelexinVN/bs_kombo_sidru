/***************************************************************
sidru_plate.scad
Программа генерации 3д модели основания для компактного варианта
СветоИзлучающего Декоративного Радиоуправляемого Устройства
(СИДРУ) проекта КомБО (Открытые системы беспроводной коммуникации)

Автор: MelexinVN (Мелехин В.Н.)
***************************************************************/

$fn = 100;          //количество полигонов

edge_height = 10;   //высота стенки
thickness = 4;      //толщина основания
ext_diameter = 68;  //диаметр основания
int_diameter = 2;   //диаметр внутренних отверстий

difference()    //Вычитаем из основания объединение тел, формирующих отверстия
{
    union()     //объединяем дно и стенку, формируем основание
    {
        translate([0,0,edge_height/2])  
        {  
            difference()    //разность цилиндров, формирующих стенку
            {
                cylinder(h = edge_height+thickness, d = ext_diameter+2, center = true); 
                cylinder(h = edge_height+thickness+1, d = ext_diameter, center = true); 
            }
        }
        //цилиндр, формирующий дно
        cylinder(h = thickness, d = ext_diameter, center = true); 
    }
    union()     //объединяем тела, формирующие отверстия
    {
     //Ряд смещенных на определенное расстояние цилиндров, задающих отверстия для крепежа
        translate([26,18,0])
            cylinder(h = thickness+1, d = int_diameter, center = true);
        translate([26,-18,0])
            cylinder(h = thickness+1, d = int_diameter, center = true);
        translate([-26,18,0])
            cylinder(h = thickness+1, d = int_diameter, center = true);
        translate([-26,-18,0])
            cylinder(h = thickness+1, d = int_diameter, center = true);
      //Параллелепипеды, формирующие прямоугольные вырезы
        translate([ext_diameter/2,0,(edge_height+thickness)/2])
            //вырез под выключатель
            cube([15,15,edge_height],center = true);
        translate([0,ext_diameter/2,(edge_height+thickness)/2])
            //вырез под usb-разъем контроллера заряда
            cube([10,10,4],center = true);
    }
}