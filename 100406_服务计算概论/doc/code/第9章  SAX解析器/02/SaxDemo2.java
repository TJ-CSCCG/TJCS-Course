import org.xml.sax.helpers.DefaultHandler;
import org.xml.sax.*;
import javax.xml.parsers.*;

public class SaxDemo2 {
    public static void main(String[] args) {
        try {
            SAXParserFactory factory = SAXParserFactory.newInstance();
            SAXParser saxParser = factory.newSAXParser();
            MyHandler myHandler = new MyHandler();
            saxParser.parse("02.xml",myHandler);
        }  catch (Exception e) {
            e.printStackTrace();
        }

    }
}
class MyHandler extends DefaultHandler{
    long startTime ; 
    public void startDocument() throws SAXException {
        startTime = System.currentTimeMillis();
        System.out.println("��ʼ����MXL�ļ�������ʼ�¼���");
        System.out.println("���԰����Լ����뷨д��������");
        System.out.println("��ʼ�¼��������\n");
    }

    public void endDocument() throws SAXException {
        System.out.println("��������¼���");
        System.out.println("���԰����Լ����뷨д��������");
        System.out.println("�����¼�������ϣ������ļ�����");
        System.out.println("�����ļ�����ʱ�䣺"+(System.currentTimeMillis()-startTime)+"����");
    }
}
