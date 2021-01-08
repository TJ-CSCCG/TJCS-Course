import org.w3c.dom.Document;

import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.parsers.DocumentBuilder;
import java.util.Scanner;
import java.io.File;

public class XMLValidate {
    public static void main(String[] args) {
        String file=null;
        try{
            Scanner reader= new Scanner(System.in);
            System.out.println("����Ҫ������ļ�����");
            file = reader.nextLine();
            DocumentBuilderFactory factory =  DocumentBuilderFactory.newInstance();
            factory.setValidating(true);
            DocumentBuilder builder = factory.newDocumentBuilder();
            MyHandler handler = new MyHandler();
            builder.setErrorHandler(handler);
            Document document = builder.parse(new File(file));
            if(handler.em==null){
                System.out.println("�ļ�"+file+"����Ч��XML�ļ���");
            }else{
                System.out.println("�ļ�"+file+"����Ч��XML�ļ���");
            }
        }catch(Exception e){
            System.out.println(e);
        }
    }
}
