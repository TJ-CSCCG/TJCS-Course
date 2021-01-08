<?xml version="1.0" encoding="gb2312" ?>
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/TR/WD-xsl">
<xsl:template match="/">
  <html>
     <body>
       <h1><xsl:value-of select="root" /></h1>
       <h2>
          今天是:<xsl:eval language="javascript">date()</xsl:eval>
          您在<xsl:eval>time()</xsl:eval>打开此页面。
       </h2>
     </body>
  </html>
  <xsl:script language="javascript">
     function date(){
        today=new Date();
        month=today.getMonth()+1;
        day=today.getDate();
        year=today.getYear();
        return year+"年"+month+"月"+day+"日";
     }
  </xsl:script>
  <xsl:script>
     function time(){
        now=new Date();
        hour=now.getHours();
        minute=now.getMinutes();
        second=now.getSeconds();
        return hour+"时"+minute+"分"+second+"秒";
     }
  </xsl:script>
</xsl:template>
</xsl:stylesheet>