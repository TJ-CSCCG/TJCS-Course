#include <Windows.h>
#include <stdlib.h>
#include <string>
#include <mysql.h> 
#include <iostream>

#include <vector>
using namespace std;
typedef vector<char *> ColName; 
int main()
{
	/*-------------------------------------
	 *   ´æ´¢ÒªÁ¬½ÓÊý¾Ý¿âµÄ±ØÒªÐÅÏ¢       *
	--------------------------------------*/

    const char user[] = "root";          //username
    const char pswd[] = "940923";        //password
    const char host[] = "localhost";     //or"127.0.0.1"
    const char table[] = "course_ex";        //database
    unsigned int port = 3306;            //server port        

    //½¨Á¢Ò»¸ömysql¶ÔÏó
	MYSQL mytest;
	//³õÊ¼»¯Êý¾Ý¿â¶ÔÏó
	mysql_init(&mytest);
		
	ColName array1;

    int res;

    if(mysql_real_connect(&mytest ,host ,user ,pswd ,table ,port ,NULL ,0))
		//Á¬½ÓÊý¾Ý¿â²¢ÇÒÔÚ³É¹¦Á¬½ÓµÄÇé¿öÏÂ½øÐÐ²Ù×÷
    {
        cout<<"Á¬½Ó" << host << "-" << table << "³É¹¦" <<endl;

        mysql_query(&mytest, "SET NAMES GBK"); //ÉèÖÃ±àÂë¸ñÊ½

		string tmpquery = "";

		char query[100] = "";

		MYSQL_ROW sql_row;//±£´æÐÐÐÅÏ¢

		MYSQL_FIELD *fd;//±£´æÁÐÐÅÏ¢
    
		do
		{
			cout << "ÊäÈë mysql Óï¾ä :" << endl;
			getline(cin ,tmpquery);

			//ÇëÇóÓï¾äµÄÊäÈë
			int i = 0 ,j = 0;
			while(i < tmpquery.length())
			{
				query[i] = tmpquery[i];
				i ++;
			}

			//±£´æÇëÇó½á¹ûµÄ±äÁ¿
			MYSQL_RES *dynamic_query;

			res = mysql_query(&mytest ,query);						
			if(!res)
			{
				cout << "ÇëÇóÒÑ³É¹¦Ö´ÐÐ" << endl;
				if(tmpquery.substr(0,6) == "select")
				{
					dynamic_query = mysql_store_result(&mytest);
					if(dynamic_query)
					{					
						for(i = 0 ;fd = mysql_fetch_field(dynamic_query) ;i++)//»ñÈ¡ÁÐÃû(ÀàËÆÒÆ¶¯ÓÎ±ê)
							array1.push_back(fd->name);

						j = mysql_num_fields(dynamic_query);   

						while(sql_row = mysql_fetch_row(dynamic_query))//»ñÈ¡¾ßÌåµÄÊý¾Ý
						{
							for(i = 0;i < j; i++)
							{
								cout << array1[i]  << "-" << sql_row[i] << endl;
                                ´ÓmysqlÇëÇó·µ»ØµÄ½á¹û¶¼ÊÇÒÔ×Ö·û´®ÐÎÊ½±£´æµÄ
								string tmptest = sql_row[i];

							}                   
						}
						if(dynamic_query != NULL) 
							mysql_free_result(dynamic_query);
					}
					else					
						cout << "²éÑ¯Êý¾Ý³ö´í" << endl;		
				}
				else
				{
					cout << "²Ù×÷Íê³É!" << endl;
				}				
			}
			else if(query[0] == '0')
			{
				cout << "¼´½«ÍË³ö!" << endl;
				break;
			}
			else
			{
				cout << "ÇëÇóÖ´ÐÐÊ§°Ü" << endl;
			}
			res = 0;
		}while(1);
	}
	else
		cout << "Êý¾Ý¿âÁ¬½ÓÊ§°Ü,¼´½«ÍË³ö£¡" << endl;
    mysql_close(&mytest);//¶Ï¿ªÁ¬½Ó
    return 0;
}
