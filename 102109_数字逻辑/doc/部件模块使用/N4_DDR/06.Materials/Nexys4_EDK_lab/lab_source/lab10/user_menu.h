/*
 * user_menu.h
 *
 *  Created on: 2013-12-2
 *      Author: ForeverHyjal
 */

#ifndef USER_MENU_H_
#define USER_MENU_H_


void Menu_main();
void Menu_vga();
void Meun_vga_sin();
void Menu_vga_free();
void Menu_vga_xy();
void Menu_vga_pw();


void Menu_vga()
{
	print("\r\n");
	print("VGA\r\n");
	print("a.sin\r\n");
	print("b.free\r\n");
	print("c.yicihanshu\r\n");
	print("d.paowuxian\r\n");
	print("q.quit\r\n");
	print(">");
}
void Menu_vga_sin()
{
	print("\r\n");
	print("VGA_Sin\r\n");
	print("w.fangdafudu\r\n");
	print("s.suoxiaofudu\r\n");
	print("a.fangdazhouqi\r\n");
	print("d.suoxiaozhouqi\r\n");
	print("\r\n");
	print("j.pingyi zuo\r\n");
	print("k.pingyi you\r\n");
	print("i.pingyi shang\r\n");
	print("m.pingyi xia\r\n");
	print("q.quit\r\n");
	print(">");
}

void Menu_vga_free()
{
	print("\r\n");
	print("VGA_Free\r\n");
	print("w.shang\r\n");
	print("s.xia\r\n");
	print("q.quit\r\n");
	print(">");
}

void Menu_vga_xy()
{
	print("\r\n");
	print("VGA_XY  y=ax+b\r\n");
	print("a.a  # jieshu\r\n");
	print("b.b  # jieshu\r\n");
	print("c.xianshi\r\n");
	print("q.quit\r\n");
	print(">");
}

void Menu_vga_pw()
{
	print("\r\n");
	print("VGA_paowuxian\r\n");
	print("a. V  #jieshu\r\n");
	print("b.xianshi\r\n");
	print("q.quit\r\n");
	print(">");
}
#endif /* USER_MENU_H_ */
