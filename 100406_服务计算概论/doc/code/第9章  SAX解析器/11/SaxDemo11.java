import org.xml.sax.helpers.DefaultHandler;
import org.xml.sax.*;
import javax.xml.parsers.*;

public class SaxDemo11 {
    public static void main(String[] args) {
        try {
            SAXParserFactory factory = SAXParserFactory.newInstance();
            factory.setValidating(true);
            SAXParser saxParser = factory.newSAXParser();
            MyHandler myHandler = new MyHandler();
            saxParser.parse("11.xml", myHandler);
        } catch (Exception e) {
            e.printStackTrace();
        }

    }
}

class MyHandler extends DefaultHandler {
    boolean flag = false;
    int line,column;
    public void startDocument() throws SAXException {
        System.out.println("��ʼ����MXL�ļ���");
    }

    public void endDocument() throws SAXException {
        System.out.println("�����ļ�����");
    }

    public void startElement(String uri, String localName, String qName, Attributes attributes) throws SAXException {
        System.out.println("<"+qName+">");
    }

    public void endElement(String uri, String localName, String qName) throws SAXException {
        System.out.println("</"+qName+">");
    }

    public void characters(char ch[], int start, int length) throws SAXException {
       System.out.println(new String(ch,start,length));
    }

    public void warning(SAXParseException e) throws SAXException {
        line = e.getLineNumber();
        column = e.getColumnNumber();
        System.out.println("���棺"+e.getMessage()+"λ�ã���"+line+"��"+column);
    }

    public void error(SAXParseException e) throws SAXException {
        line = e.getLineNumber();
        column = e.getColumnNumber();
        System.out.println("һ�����"+e.getMessage()+"λ�ã���"+line+"��"+column);
    }

    public void fatalError(SAXParseException e) throws SAXException {
        line = e.getLineNumber();
        column = e.getColumnNumber();
        System.out.println("���ش���"+e.getMessage()+"λ�ã���"+line+"��"+column);
        throw new SAXException("�ļ��з����������󣬽����޷����");
    }
}

