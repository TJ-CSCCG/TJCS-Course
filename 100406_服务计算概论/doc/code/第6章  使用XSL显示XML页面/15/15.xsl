<?xml version="1.0" encoding="gb2312" ?>
<xsl:stylesheet xmlns:xsl="http://www.w3.org/TR/WD-xsl">
<xsl:template math="/">
	<html>
	<body>
		<center>
		<table border="1" width="300" >
		<tr>
			<td align="center">id</td>
			<td align="center">name</td>
		</tr>
		<tr>
			<td align="center"><xsl:value-of select="/message/persion[@id$eq$'2']/@id" /></td>
		        <td align="center"><xsl:value-of select="/message/persion[@id$eq$'2']/name" /></td>
                </tr>
		</table>
		</center>
	</body>
	</html>
</xsl:template>
</xsl:stylesheet>