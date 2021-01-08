import org.xml.sax.helpers.DefaultHandler;
import org.xml.sax.*;
import javax.xml.parsers.*;

public class SaxDemo7 {
    public static void main(String[] args) {
        try {
            SAXParserFactory factory = SAXParserFactory.newInstance();
            factory.setNamespaceAware(true);
            SAXParser saxParser = factory.newSAXParser();
            MyHandler myHandler = new MyHandler();
            saxParser.parse("07.xml", myHandler);
        } catch (Exception e) {
            e.printStackTrace();
        }

    }
}

class MyHandler extends DefaultHandler {
    int n=0;
    public void startDocument() throws SAXException {
        System.out.println("��ʼ����MXL�ļ���");
    }

    public void endDocument() throws SAXException {
        System.out.println("�����ļ�����");
        System.out.println("�ļ�����"+n+"�����ƿռ䡣");
    }

    public void startElement(String uri, String localName, String qName, Attributes attributes) throws SAXException {
        System.out.print("<"+qName+">");
    }

    public void endElement(String uri, String localName, String qName) throws SAXException {
        System.out.print("</"+qName+">");
    }

    public void characters(char ch[], int start, int length) throws SAXException {
        System.out.print(new String(ch,start,length));
    }

    public void startPrefixMapping(String prefix, String uri) throws SAXException {
        System.out.println("��"+(++n)+"�����ƿռ䣺");
        System.out.println("ǰ׺��"+prefix+"���ƣ�"+uri);
    }

    public void endPrefixMapping(String prefix) throws SAXException {
        System.out.print("\nǰ׺Ϊ"+prefix+"�����ƿռ������������");
    }

}

