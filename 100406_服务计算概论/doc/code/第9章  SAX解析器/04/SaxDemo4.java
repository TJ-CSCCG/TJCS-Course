import org.xml.sax.helpers.DefaultHandler;
import org.xml.sax.*;
import javax.xml.parsers.*;

public class SaxDemo4 {
    public static void main(String[] args) {
        try {
            SAXParserFactory factory = SAXParserFactory.newInstance();
            factory.setNamespaceAware(true);
            SAXParser saxParser = factory.newSAXParser();
            MyHandler myHandler = new MyHandler();
            saxParser.parse("04.xml", myHandler);
        } catch (Exception e) {
            e.printStackTrace();
        }

    }
}

class MyHandler extends DefaultHandler {
    long startTime;
    int n=0;
    public void startDocument() throws SAXException {
        startTime = System.currentTimeMillis();
        System.out.println("��ʼ����MXL�ļ���");
    }

    public void endDocument() throws SAXException {
        System.out.println("�����ļ�����");
        System.out.println("�����ļ�����ʱ�䣺" + (System.currentTimeMillis() - startTime) + "����");
    }

    public void startElement(String uri, String localName, String qName, Attributes attributes) throws SAXException {
        System.out.println((++n)+".\""+localName+"\"��ǣ���ǿ�ʼ�¼�");
        System.out.print("  ������ƣ�" + localName + "��");
        System.out.print("���ƿռ䣺" + uri + "��");
        System.out.println("������ƣ�" + qName + "��");
        if (attributes.getLength() > 0) {
            System.out.print("  �������ԣ�");
            for (int i = 0; i < attributes.getLength(); i++) {
                System.out.print( attributes.getLocalName(i));
                System.out.println(" = " + attributes.getValue(i)+";");
            }
        }else{
            System.out.println("  �������ԣ�null");
        }
    }

    public void endElement(String uri, String localName, String qName) throws SAXException {
        System.out.println("\""+localName+"\"��ǽ����¼�");
    }
}
