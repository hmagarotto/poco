//
// Preparation.cpp
//
// $Id: //poco/Main/Data/ODBC/src/Preparation.cpp#5 $
//
// Library: Data
// Package: DataCore
// Module:  Preparation
//
// Copyright (c) 2006, Applied Informatics Software Engineering GmbH.
// and Contributors.
//
// Permission is hereby granted, free of charge, to any person or organization
// obtaining a copy of the software and accompanying documentation covered by
// this license (the "Software") to use, reproduce, display, distribute,
// execute, and transmit the Software, and to prepare derivative works of the
// Software, and to permit third-parties to whom the Software is furnished to
// do so, all subject to the following:
// 
// The copyright notices in the Software and this entire statement, including
// the above license grant, this restriction and the following disclaimer,
// must be included in all copies of the Software, in whole or in part, and
// all derivative works of the Software, unless such copies or derivative
// works are solely in the form of machine-executable object code generated by
// a source language processor.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
// SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
// FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
// ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.
//


#include "Poco/Data/ODBC/Preparation.h"
#include "Poco/Data/ODBC/ODBCColumn.h"


namespace Poco {
namespace Data {
namespace ODBC {


Preparation::Preparation(const StatementHandle& rStmt, 
	const std::string& statement, 
	std::size_t maxFieldSize,
	DataExtraction dataExtraction): 
	_rStmt(rStmt),
	_maxFieldSize(maxFieldSize),
	_dataExtraction(dataExtraction)
{
	POCO_SQLCHAR* pStr = (POCO_SQLCHAR*) statement.c_str();
	if (Utility::isError(SQLPrepare(_rStmt, pStr, (SQLINTEGER) statement.length())))
		throw StatementException(_rStmt);
}


Preparation::~Preparation()
{
	std::vector<SQLLEN*>::iterator itLen = _pLengths.begin();
	std::vector<SQLLEN*>::iterator itLenEnd = _pLengths.end();
	for (; itLen != itLenEnd; ++itLen) delete *itLen;

	std::vector<Poco::Any*>::iterator itVal = _pValues.begin();
	std::vector<Poco::Any*>::iterator itValEnd = _pValues.end();
	for (; itVal != itValEnd; ++itVal) delete *itVal;
}


std::size_t Preparation::columns() const
{
	if (_pValues.empty())
	{
		SQLSMALLINT nCol = 0;
		if (!Utility::isError(SQLNumResultCols(_rStmt, &nCol)) && 
			0 != nCol)
		{
			_pValues.resize(nCol, 0);
			_pLengths.resize(nCol, 0);
		}
	}

	return _pValues.size();
}


Poco::Any& Preparation::operator [] (std::size_t pos)
{
	poco_assert (pos >= 0 && pos < _pValues.size());
	
	return *_pValues[pos];
}


void Preparation::prepare(std::size_t pos, const Poco::Any&)
{
	ODBCColumn col(_rStmt, pos);

	switch (col.type())
	{
		case MetaColumn::FDT_INT8:
			return preparePOD<Poco::Int8>(pos, SQL_C_STINYINT); 

		case MetaColumn::FDT_UINT8:
			return preparePOD<Poco::UInt8>(pos, SQL_C_UTINYINT);

		case MetaColumn::FDT_INT16:
			return preparePOD<Poco::Int16>(pos, SQL_C_SSHORT);

		case MetaColumn::FDT_UINT16:
			return preparePOD<Poco::UInt16>(pos, SQL_C_USHORT);

		case MetaColumn::FDT_INT32:
			return preparePOD<Poco::Int32>(pos, SQL_C_SLONG);

		case MetaColumn::FDT_UINT32:
			return preparePOD<Poco::UInt32>(pos, SQL_C_ULONG);

		case MetaColumn::FDT_INT64:
			return preparePOD<Poco::Int64>(pos, SQL_C_SBIGINT);

		case MetaColumn::FDT_UINT64:
			return preparePOD<Poco::UInt64>(pos, SQL_C_UBIGINT);

		case MetaColumn::FDT_BOOL:
			return preparePOD<bool>(pos, SQL_C_BIT);

		case MetaColumn::FDT_FLOAT:
			return preparePOD<float>(pos, SQL_C_FLOAT);

		case MetaColumn::FDT_DOUBLE:
			return preparePOD<float>(pos, SQL_C_DOUBLE);

		case MetaColumn::FDT_STRING:
			return prepareRaw<char>(pos, SQL_C_CHAR, maxDataSize(pos));

		case MetaColumn::FDT_BLOB:
			return prepareRaw<char>(pos, SQL_C_BINARY, maxDataSize(pos));

		default: 
			throw DataFormatException("Unsupported data type.");
	}
}


std::size_t Preparation::maxDataSize(std::size_t pos) const
{
	poco_assert (pos >= 0 && pos < _pValues.size());

	std::size_t sz = 0;
	std::size_t maxsz = getMaxFieldSize();

	try 
	{
		sz = ODBCColumn(_rStmt, pos).length();
	}
	catch (StatementException&) { }

	if (!sz || sz > maxsz) sz = maxsz;
	return sz;
}


} } } // namespace Poco::Data::ODBC
