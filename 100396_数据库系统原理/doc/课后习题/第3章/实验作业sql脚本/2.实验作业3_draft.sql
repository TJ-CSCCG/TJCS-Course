DROP DATABASE IF EXISTS `ADM`;
CREATE DATABASE `ADM`;
USE `ADM`;

-- table agent --
CREATE TABLE `ADM`.`Agent`(
	`AID` int NOT NULL,
    `ANAME` varchar(50) NOT NULL,
    `SALARY` double NOT NULL,

    primary key (`AID`)
) ENGINE = INNODB;

-- table product --
CREATE TABLE `ADM`.`Product`(
	`PID` int NOT NULL,
    `PNAME` varchar(50) NOT NULL,
    `PRICE` double NOT NULL,
    
    primary key (`PID`)
) ENGINE = INNODB;

-- table customer --
CREATE TABLE `ADM`.`Customer`(
	`CID` int NOT NULL,
    `CNAME` varchar(50) NOT NULL,
    
    primary key (`CID`)
) ENGINE = INNODB;

-- table orders --
CREATE TABLE `ADM`.`Order` (
	`OID` int NOT NULL,
    `BUY_DATE` datetime NOT NULL default now(),
    
    `QTY` int NOT NULL check(QTY > 0),
    `DOLLARS` double NOT NULL default 0 check(DOLLARS >= 0),
    
    `CID` int NOT NULL,
    `AID` int NOT NULL,
    `PID` int NOT NULL,
    
    primary key (`OID`),
    constraint `foreign_key_order_agent`
		foreign key(`AID`)
        references `ADM`.`Agent`(`AID`)
        on update no action
        on delete no action,        
    constraint `foreign_key_order_customer`
		foreign key(`CID`)
        references `ADM`.`Customer`(`CID`)
        on update no action
        on delete no action,
	constraint `foreign_key_order_product`
		foreign key(`PID`)
        references `ADM`.`Product`(`PID`)
        on update no action
        on delete no action
) ENGINE = INNODB;

INSERT INTO `ADM`.`Agent`(`AID`, `ANAME`, `SALARY`) VALUES (01, 'Smith', 10000);
INSERT INTO `ADM`.`Agent`(`AID`, `ANAME`, `SALARY`) VALUES(02, 'Jones', 7000);
INSERT INTO `ADM`.`Agent`(`AID`, `ANAME`, `SALARY`) VALUES(03, 'Brown', 5000);
INSERT INTO `ADM`.`Agent`(`AID`, `ANAME`, `SALARY`) VALUES(04, 'Gray', 7200);
INSERT INTO `ADM`.`Agent`(`AID`, `ANAME`, `SALARY`) VALUES(05, 'Otasi', 4800);
INSERT INTO `ADM`.`Agent`(`AID`, `ANAME`, `SALARY`) VALUES(06, 'Jack', 5500);

INSERT INTO `ADM`.`Customer`(`CID`, `CNAME`) VALUES(001, 'TipTop');
INSERT INTO `ADM`.`Customer`(`CID`, `CNAME`) VALUES(002, 'Basics');
INSERT INTO `ADM`.`Customer`(`CID`, `CNAME`) VALUES(003, 'Allied');
INSERT INTO `ADM`.`Customer`(`CID`, `CNAME`) VALUES(004, 'ACME');
INSERT INTO `ADM`.`Customer`(`CID`, `CNAME`) VALUES(005, 'ACME');

INSERT INTO `ADM`.`Product`(`PID`, `PNAME`, `PRICE`) VALUES(01, 'comb', 0.5);
INSERT INTO `ADM`.`Product`(`PID`, `PNAME`, `PRICE`) VALUES(02, 'brush', 0.5);
INSERT INTO `ADM`.`Product`(`PID`, `PNAME`, `PRICE`) VALUES(03, 'razor', 1);
INSERT INTO `ADM`.`Product`(`PID`, `PNAME`, `PRICE`) VALUES(04, 'pen', 1);
INSERT INTO `ADM`.`Product`(`PID`, `PNAME`, `PRICE`) VALUES(05, 'pencil', 1);

INSERT INTO `ADM`.`Order`(`OID`, `BUY_DATE`, `CID`, `AID`, `PID`, `QTY`, `DOLLARS`) 
	VALUES(1011, '2002-4-8', 001, 01, 01, 1000, 450);
INSERT INTO `ADM`.`Order`(`OID`, `BUY_DATE`, `CID`, `AID`, `PID`, `QTY`, `DOLLARS`) 
	VALUES(1012, '2001-4-1', 001, 02, 02, 400, 180);
INSERT INTO `ADM`.`Order`(`OID`, `BUY_DATE`, `CID`, `AID`, `PID`, `QTY`, `DOLLARS`) 
	VALUES(1013, '2002-1-1', 002, 03, 03, 1000, 880);
INSERT INTO `ADM`.`Order`(`OID`, `BUY_DATE`, `CID`, `AID`, `PID`, `QTY`, `DOLLARS`) 
	VALUES(1014, '2001-5-1', 002, 05, 03, 800, 704);
INSERT INTO `ADM`.`Order`(`OID`, `BUY_DATE`, `CID`, `AID`, `PID`, `QTY`, `DOLLARS`) 
	VALUES(1015, '2002-1-1', 003, 03, 05, 1200, 1104);
INSERT INTO `ADM`.`Order`(`OID`, `BUY_DATE`, `CID`, `AID`, `PID`, `QTY`, `DOLLARS`) 
	VALUES(1016, '2001-8-4', 004, 06, 01, 1000, 460);
INSERT INTO `ADM`.`Order`(`OID`, `BUY_DATE`, `CID`, `AID`, `PID`, `QTY`, `DOLLARS`) 
	VALUES(1017, '2002-9-1', 005, 01, 04, 1000, 500);
INSERT INTO `ADM`.`Order`(`OID`, `BUY_DATE`, `CID`, `AID`, `PID`, `QTY`, `DOLLARS`) 
	VALUES(1018, '2001-3-6', 005, 01, 01, 800, 400);
