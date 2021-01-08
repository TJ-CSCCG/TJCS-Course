import org.xml.sax.helpers.DefaultHandler;
import org.xml.sax.*;
import javax.xml.parsers.*;

public class SaxDemo9 {
    public static void main(String[] args) {
        try {
            SAXParserFactory factory = SAXParserFactory.newInstance();
            SAXParser saxParser = factory.newSAXParser();
            MyHandler myHandler = new MyHandler();
            saxParser.parse("09.xml", myHandler);
        } catch (Exception e) {
            e.printStackTrace();
        }

    }
}

class MyHandler extends DefaultHandler {
    public void startDocument() throws SAXException {
        System.out.println("��ʼ����MXL�ļ���");
    }

    public void endDocument() throws SAXException {
        System.out.println("�����ļ�����");
    }

    public void startElement(String uri, String localName, String qName, Attributes attributes) throws SAXException {
        System.out.print("<"+qName+">");
    }

    public void endElement(String uri, String localName, String qName) throws SAXException {
        System.out.println("</"+qName+">");
    }

    public void characters(char ch[], int start, int length) throws SAXException {
        System.out.println(new String(ch,start,length));
    }

    public void unparsedEntityDecl(String name, String publicId, String systemId, String notationName) throws SAXException {
        System.out.println("���ɽ�����ʵ�壺");
        System.out.println("ʵ�����ƣ�"+name);
        System.out.println("publicId��"+publicId);
        System.out.println("systemId��"+systemId);
        System.out.println("Ӧ�ó������ƣ�"+notationName);
    }

}

