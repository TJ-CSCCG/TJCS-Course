import org.xml.sax.helpers.*;
import org.xml.sax.*;
import javax.xml.parsers.*;

public class SaxDemo12 {
    public static void main(String[] args) {
        try {
            SAXParserFactory factory = SAXParserFactory.newInstance();
            SAXParser saxParser = factory.newSAXParser();
            XMLReader reader = saxParser.getXMLReader();
            Myfilter1 myfilter1 = new Myfilter1();
            Myfilter2 myfilter2 = new Myfilter2();
            myfilter1.setParent(reader);
            myfilter2.setParent(myfilter1);
            MyHandler myHandler = new MyHandler();
            myfilter2.setContentHandler(myHandler);
            myfilter2.parse("12.xml");
        } catch (Exception e) {
            e.printStackTrace();
        }

    }
}

class MyHandler extends DefaultHandler {
    
    public void startDocument() throws SAXException {
        System.out.println("开始解析MXL文件：");
    }

    public void endDocument() throws SAXException {
        System.out.println("\n解析文件结束");
    }

    public void startElement(String uri, String localName, String qName, Attributes attributes) throws SAXException {
        System.out.print("<"+qName+">");
    }
   
    public void endElement(String uri, String localName, String qName) throws SAXException {
        System.out.print("</"+qName+">");
    }

    public void characters(char ch[], int start, int length) throws SAXException {
        System.out.print(new String(ch, start, length));
    }
}

class Myfilter1 extends XMLFilterImpl {
    public void startElement(String uri, String localName, String qName, Attributes atts) throws SAXException {
        if (qName.equals("records")) {
            String newName = "成绩单";
            super.startElement(uri, localName, newName, atts);
        }
        if (qName.equals("student")) {
            String newName = "学生";
            super.startElement(uri, localName, newName, atts);
        }
        if (qName.equals("name")) {
            String newName = "姓名";
            super.startElement(uri, localName, newName, atts);
        }
        if (qName.equals("score")) {
            String newName = "成绩";
            super.startElement(uri, localName, newName, atts);
        }
    }
    public void endElement(String uri, String localName, String qName) throws SAXException {
        if (qName.equals("records")) {
            String newName = "成绩单";
            super.endElement(uri, localName, newName);
        }
        if (qName.equals("student")) {
            String newName = "学生";
            super.endElement(uri, localName, newName);
        }
        if (qName.equals("name")) {
            String newName = "姓名";
            super.endElement(uri, localName, newName);
        }
        if (qName.equals("score")) {
            String newName = "成绩";
            super.endElement(uri, localName, newName);
        }
    }
}

class Myfilter2 extends XMLFilterImpl{
   boolean flag = false;
   public void startElement(String uri, String localName, String qName, Attributes atts) throws SAXException {
        if (qName.equals("成绩")) {
            flag = true;
        }
        super.startElement(uri, localName, qName, atts);
    }
   public void endElement(String uri, String localName, String qName) throws SAXException {
        super.endElement(uri, localName, qName);
    }
   public void characters(char ch[], int start, int length) throws SAXException {
        if (flag) {
            int score = Integer.parseInt(new String(ch, start, length));
            if (score > 90) {
                ch[start] = 'A';
            } else if (score > 60) {
                ch[start] = 'B';
            } else {
                ch[start] = 'C';
            }
            super.characters(ch, start, 1);
            flag = false;
        } else {
            super.characters(ch, start, length);
        }
    }
}

