#include "sf_debug.h"

using namespace skyfire;
sf_debug<> sflog;


int main()
{
	//��������˳���ܵߵ�
	//����־����
	sflog.set_out_to_file(true);
	//������־�ļ�
	sflog.set_debug_file("runlog.txt");
	sflog.sf_logout("hello", "���Ƕ�����ʾ", 666, 5.6, 5.7f, 9L);
	sflog.sf_logout(5);
	sflog.sf_track("��ӡ�ڵ�������", 666);
}