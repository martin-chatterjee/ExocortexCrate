// alembicPlugin
// Initial code generated by Softimage SDK Wizard
// Executed Fri Aug 19 09:14:49 UTC+0200 2011 by helge
// 
// Tip: You need to compile the generated code before you can load the plug-in.
// After you compile the plug-in, you can load it by clicking Update All in the Plugin Manager.
#include "stdafx.h"
#include "arnoldHelpers.h" 

using namespace XSI; 
using namespace MATH; 

#include "AlembicLicensing.h"

#include "AlembicWriteJob.h"
#include "AlembicPoints.h"
#include "AlembicCurves.h"
#include "CommonProfiler.h"
#include "CommonMeshUtilities.h"
#include "CommonUtilities.h"

#include <set>
#include <map>
#include <string>

ESS_CALLBACK_START(alembic_get_nodes_Init,CRef&)
	Context ctxt( in_ctxt );
	Command oCmd;
	oCmd = ctxt.GetSource();
	oCmd.PutDescription(L"");
	oCmd.EnableReturnValue(true);

	return CStatus::OK;
ESS_CALLBACK_END

ESS_CALLBACK_START(alembic_get_nodes_Execute, CRef&)

   Context ctxt( in_ctxt );
   CString search = L"*.*.*.alembic_*,*.*.*.ABC_*";
   search += L",*.*.alembic_*,*.*.ABC_*";
   search += L",*.*.cls.Texture_Coordinates_AUTO.*.alembic*";
   
   CComAPIHandler collection;
   collection.CreateInstance(L"XSI.Collection");
   collection.PutProperty(L"Unique", true);
   CValue colReturnVal;
   collection.Call(L"SetAsText", colReturnVal, search);
   std::set<CRef> refs;//refs to all alembic nodes (nodes that have alembic operators or alembic ice nodes as children)
   for(LONG i=0;i<(LONG)collection.GetProperty(L"Count");i++)
   {
      CValueArray apiArgs;
      apiArgs.Add(i);
      collection.Invoke( L"Item", CComAPIHandler::PropertyGet, colReturnVal, apiArgs );
      CRef ref = colReturnVal;
      if(!ref.IsValid())
         continue;
      ICETree tree(ref);
      if(tree.IsValid())
      {
         CRefArray compounds = tree.GetCompoundNodes();
         for(LONG j=0;j<compounds.GetCount();j++)
         {
            ICECompoundNode compound(compounds[j]);
            if(compound.GetFullName() == tree.GetFullName()){
               continue;
            }
            if(compound.GetName().GetSubString(0,8).IsEqualNoCase(L"abc load")){
               //refs.Add(compound.GetRef());
               refs.insert(compound.GetParent3DObject().GetRef());
            }
         }
         continue;
      }
      CustomOperator op(ref);
      if(op.IsValid())
      {
         //refs.Add(ref);
         refs.insert(op.GetParent3DObject().GetRef());
         continue;
      }
   }

   CValueArray strVals;
   for(std::set<CRef>::iterator it=refs.begin(); it != refs.end(); it++)
   {
      X3DObject xObj(*it);
      CString name = xObj.GetName();
      //CString fullname = xObj.GetFullName();
      CString modelname = xObj.GetModel().GetName();
      //ESS_LOG_WARNING("name: "<<fullname.GetAsciiString());
      strVals.Add(modelname+"."+name);
   }

   CValue returnVal(strVals);
   ctxt.PutAttribute(L"ReturnValue", returnVal);

   return CStatus::OK;
ESS_CALLBACK_END


ESS_CALLBACK_START(alembic_get_paths_Init,CRef&)
	Context ctxt( in_ctxt );
	Command oCmd;
	oCmd = ctxt.GetSource();
	oCmd.PutDescription(L"");
	oCmd.EnableReturnValue(true);

	return CStatus::OK;
ESS_CALLBACK_END

ESS_CALLBACK_START(alembic_get_paths_Execute, CRef&)

   Context ctxt( in_ctxt );

   std::vector<std::string> paths;
   getPaths(paths);

   CValueArray strVals;

   for(int i=0; i<paths.size(); i++)
   {
      strVals.Add(paths[i].c_str());
   }

   CValue returnVal(strVals);
   ctxt.PutAttribute(L"ReturnValue", returnVal);

   return CStatus::OK;
ESS_CALLBACK_END



ESS_CALLBACK_START(alembic_replace_path_Init,CRef&)
	Context ctxt( in_ctxt );
	Command oCmd;
	oCmd = ctxt.GetSource();
	oCmd.PutDescription(L"");
	oCmd.EnableReturnValue(true);

	ArgumentArray oArgs;
	oArgs = oCmd.GetArguments();
    oArgs.Add(L"OldPath");
	oArgs.Add(L"NewPath");

	return CStatus::OK;
ESS_CALLBACK_END

ESS_CALLBACK_START(alembic_replace_path_Execute, CRef&)

   Context ctxt( in_ctxt );
   CValueArray args = ctxt.GetAttribute(L"Arguments");
   CString ooldPath = args[0];
   CString newPath = args[1];
   
   CString oldPath;
  
   //Assume that if we find a single back slash, it was not intended to be the escape character
   for(unsigned i=0; i<ooldPath.Length(); i++){
      char c = ooldPath.GetAt(i);
      if( c == '\n' || c == '\t' || c == '\\' || c == '\b' || c == '\r' || c == '\f'){
         oldPath += '\\';
      }
      else{
         oldPath += c;
      }
   }

  // ESS_LOG_WARNING("oldPath: "<<oldPath.GetAsciiString());
  // ESS_LOG_WARNING("newPath: "<<newPath.GetAsciiString());

   CString search = L"*.*.*.alembic_*,*.*.*.ABC_*";
   search += L",*.*.alembic_*,*.*.ABC_*";
   search += L",*.*.cls.Texture_Coordinates_AUTO.*.alembic*";
   
   CComAPIHandler collection;
   collection.CreateInstance(L"XSI.Collection");
   collection.PutProperty(L"Unique",true);
   CValue returnVal;
   collection.Call(L"SetAsText",returnVal,search);
   CRefArray refs;//an array of refs to all alembic operators and ice nodes
   for(LONG i=0;i<(LONG)collection.GetProperty(L"Count");i++)
   {
      CValueArray apiArgs;
      apiArgs.Add(i);
      collection.Invoke( L"Item", CComAPIHandler::PropertyGet, returnVal, apiArgs );
      CRef ref = returnVal;
      if(!ref.IsValid())
         continue;
      ICETree tree(ref);
      if(tree.IsValid())
      {
         CRefArray compounds = tree.GetCompoundNodes();
         for(LONG j=0;j<compounds.GetCount();j++)
         {
            ICECompoundNode compound(compounds[j]);
            if(compound.GetFullName() == tree.GetFullName())
               continue;
            if(compound.GetName().GetSubString(0,8).IsEqualNoCase(L"abc load"))
               refs.Add(compound.GetRef());
         }
         continue;
      }
      CustomOperator op(ref);
      if(op.IsValid())
      {
         refs.Add(ref);
         continue;
      }
   }



   int nReplaceCount = 0;

   for(LONG i=0;i<refs.GetCount();i++)
   {
      std::string values[4];
      ICECompoundNode tree(refs[i]);
      if(tree.IsValid())
      {
         Parameter path = ICENodeInputPort(tree.GetInputPorts().GetItem(L"path")).GetParameters()[0];
         CString pathStr = path.GetValue();
         //ESS_LOG_WARNING("path: "<<pathStr.GetAsciiString());
         if( pathStr.IsEqualNoCase(oldPath) ){
            path.PutValue(newPath);

            delRefArchive(oldPath);
            addRefArchive(newPath);

            nReplaceCount++;
         }
      }
      else
      {
         CustomOperator op(refs[i]);
         if(op.IsValid())
         {
            CString pathStr = op.GetParameterValue(L"path");
            //ESS_LOG_WARNING("path: "<<pathStr.GetAsciiString());
            if( pathStr.IsEqualNoCase(oldPath) ){
               op.PutParameterValue(L"path", newPath);

               delRefArchive(oldPath);
               addRefArchive(newPath);

               nReplaceCount++;
            }
         }
      }
   }

   ESS_LOG_WARNING(nReplaceCount<<" paths replaced.");

   return CStatus::OK;
ESS_CALLBACK_END


ESS_CALLBACK_START(alembic_path_manager_Init,CRef&)
	Context ctxt( in_ctxt );
	Command oCmd;
	oCmd = ctxt.GetSource();
	oCmd.PutDescription(L"");
	oCmd.EnableReturnValue(true);

	ArgumentArray oArgs;
	oArgs = oCmd.GetArguments();
	oArgs.Add(L"model");
	return CStatus::OK;
ESS_CALLBACK_END

ESS_CALLBACK_START(alembic_path_manager_Execute, CRef&)

	Context ctxt( in_ctxt );
	CValueArray args = ctxt.GetAttribute(L"Arguments");
   CString model = args[0];
   CRefArray allRefs;
   CString search;
   if(model.IsEmpty())
   {
      search = L"*.*.*.alembic_*,*.*.*.ABC_*";
      search += L",*.*.alembic_*,*.*.ABC_*";
	  search += L",*.*.cls.Texture_Coordinates_AUTO.*.alembic*";
   }
   else
      search = model+L".*.*.alembic_*,"+model+"L.*.*.ABC_*";

   CComAPIHandler collection;
   collection.CreateInstance(L"XSI.Collection");
   collection.PutProperty(L"Unique",true);
   CValue returnVal;
   collection.Call(L"SetAsText",returnVal,search);
   CRefArray refs;//an array of refs to all alembic operators and ice nodes
   for(LONG i=0;i<(LONG)collection.GetProperty(L"Count");i++)
   {
      CValueArray apiArgs;
      apiArgs.Add(i);
      collection.Invoke( L"Item", CComAPIHandler::PropertyGet, returnVal, apiArgs );
      CRef ref = returnVal;
      if(!ref.IsValid())
         continue;
      ICETree tree(ref);
      if(tree.IsValid())
      {
         CRefArray compounds = tree.GetCompoundNodes();
         for(LONG j=0;j<compounds.GetCount();j++)
         {
            ICECompoundNode compound(compounds[j]);
            if(compound.GetFullName() == tree.GetFullName())
               continue;
            if(compound.GetName().GetSubString(0,8).IsEqualNoCase(L"abc load"))
               refs.Add(compound.GetRef());
         }
         continue;
      }
      CustomOperator op(ref);
      if(op.IsValid())
      {
         refs.Add(ref);
         continue;
      }
   }


   //build a map of all identifiers and a map of all paths
   stringMap pathMap;
   stringMap identifierMap;
   stringMap * maps[4];
   maps[0] = &pathMap;
   maps[1] = &pathMap;
   maps[2] = &identifierMap;
   maps[3] = &identifierMap;
   CStringArray paramName;
   paramName.Add(L"path");
   paramName.Add(L"renderpath");
   paramName.Add(L"identifier");
   paramName.Add(L"renderidentifier");

   for(LONG i=0;i<refs.GetCount();i++)
   {
      std::string values[4];
      ICECompoundNode tree(refs[i]);
      if(tree.IsValid())
      {
         for(LONG j=0;j<paramName.GetCount();j++)
         {
            ICENodeInputPort port = tree.GetInputPorts().GetItem(paramName[j]);
            Parameter param = port.GetParameters()[0];
            values[j] = CString(param.GetValue()).GetAsciiString();
         }
      }
      else
      {
         CustomOperator op(refs[i]);
         if(op.IsValid())
         {
            for(LONG j=0;j<paramName.GetCount();j++)
               values[j] = CString(op.GetParameterValue(paramName[j])).GetAsciiString();
         }
      }

      for(LONG j=0;j<paramName.GetCount();j++)
         if(maps[j]->find(values[j]) == maps[j]->end() && !values[j].empty())
            maps[j]->insert(stringPair(values[j],values[j]));
   }

   if(pathMap.size() == 0 && identifierMap.size() == 0)
   {
      Application().LogMessage(L"[ExocortexAlembic] No alembic operators / icetrees found!",siWarningMsg);
      return CStatus::OK;
   }

   // awesome - construct the custom property
   CustomProperty prop = (CustomProperty) Application().GetActiveSceneRoot().AddProperty(L"CustomProperty");
   prop.PutName(L"options");
   PPGLayout layout = prop.GetPPGLayout();
   layout.Clear();

   // add the search and replace stuff
	Parameter oParam;
   LONG pathCount = 0;
   LONG identifierCount = 0;
   if(pathMap.size() > 0)
   {
      prop.AddParameter(L"path_search",CValue::siString,siPersistable,L"",L"",L"",L"",L"",L"",L"",oParam);
      prop.AddParameter(L"path_replace",CValue::siString,siPersistable,L"",L"",L"",L"",L"",L"",L"",oParam);
      layout.AddTab(L"Paths");
      layout.AddGroup(L"Search and Replace");
      layout.AddItem(L"path_search").PutAttribute(siUINoLabel,true);
      layout.AddItem(L"path_replace").PutAttribute(siUINoLabel,true);
      layout.AddButton(L"path_button0",L"Replace All");
      layout.EndGroup();

      for(stringMapIt it = pathMap.begin(); it != pathMap.end(); it++)
      {
         prop.AddParameter(L"_path_"+CString(pathCount),CValue::siString,siPersistable,L"",L"",it->second.c_str(),L"",L"",L"",L"",oParam);
         layout.AddItem(L"_path_"+CString(pathCount)).PutAttribute(siUINoLabel,true);
         pathCount++;
      }
   }
   if(identifierMap.size() >  0)
   {
      prop.AddParameter(L"identifier_search",CValue::siString,siPersistable,L"",L"",L"",L"",L"",L"",L"",oParam);
      prop.AddParameter(L"identifier_replace",CValue::siString,siPersistable,L"",L"",L"",L"",L"",L"",L"",oParam);
      layout.AddTab(L"Identifiers");
      layout.AddGroup(L"Search and Replace");
      layout.AddItem(L"identifier_search").PutAttribute(siUINoLabel,true);
      layout.AddItem(L"identifier_replace").PutAttribute(siUINoLabel,true);
      layout.AddButton(L"identifier_button0",L"Replace All");
      layout.EndGroup();

      for(stringMapIt it = identifierMap.begin(); it != identifierMap.end(); it++)
      {
         prop.AddParameter(L"_identifier_"+CString(identifierCount),CValue::siString,siPersistable,L"",L"",it->second.c_str(),L"",L"",L"",L"",oParam);
         layout.AddItem(L"_identifier_"+CString(identifierCount)).PutAttribute(siUINoLabel,true);
         identifierCount++;
      }
   }

   // construct the logic
   layout.PutLanguage(L"JScript");
   CString logic;
   logic += L"function path_button0_OnClicked() {\n";
   logic += L"  var prop = PPG.Inspected(0);\n";
   logic += L"  var search = prop.parameters('path_search').value;\n";
   logic += L"  var replace = prop.parameters('path_replace').value;\n";
   logic += L"  if(!search){\n";
   logic += L"    LogMessage('[ExocortexAlembic] No search string specified!',siWarningMsg);\n";
   logic += L"    return;\n";
   logic += L"  }\n";
   logic += L"  for(var i=0;i<prop.parameters.count;i++){\n";
   logic += L"    var name = prop.parameters(i).scriptname;\n";
   logic += L"    if(name.substr(0,6) != '_path_')continue;\n";
   logic += L"    var value = prop.parameters(i).value;\n";
   logic += L"    value = value.replace(search,replace);\n";
   logic += L"    prop.parameters(i).value = value;\n";
   logic += L"  }\n";
   logic += L"}\n";
   logic += L"function identifier_button0_OnClicked() {\n";
   logic += L"  var prop = PPG.Inspected(0);\n";
   logic += L"  var search = prop.parameters('identifier_search').value;\n";
   logic += L"  var replace = prop.parameters('identifier_replace').value;\n";
   logic += L"  if(!search){\n";
   logic += L"    LogMessage('[ExocortexAlembic] No search string specified!',siWarningMsg);\n";
   logic += L"    return;\n";
   logic += L"  }\n";
   logic += L"  for(var i=0;i<prop.parameters.count;i++){\n";
   logic += L"    var name = prop.parameters(i).scriptname;\n";
   logic += L"    if(name.substr(0,12) != '_identifier_')continue;\n";
   logic += L"    var value = prop.parameters(i).value;\n";
   logic += L"    value = value.replace(search,replace);\n";
   logic += L"    prop.parameters(i).value = value;\n";
   logic += L"  }\n";
   logic += L"}\n";
   layout.PutLogic(logic);

   CValueArray inspectArgs(5);
   CValue inspectResult;
   inspectArgs[0] = prop.GetFullName();
   inspectArgs[1] = L"";
   inspectArgs[2] = L"Path Manager";
   inspectArgs[3] = siModal;
   inspectArgs[4] = false;
   Application().ExecuteCommand(L"InspectObj",inspectArgs,inspectResult);
   
   // prepare for deletion
   if(!inspectResult)
   {
      // read all of the values
      pathCount = identifierCount = 0;
      for(stringMapIt it = pathMap.begin(); it != pathMap.end(); it++)
      {
         CString value = prop.GetParameterValue(L"_path_"+CString(pathCount++));
         it->second = value.GetAsciiString();
      }
      for(stringMapIt it = identifierMap.begin(); it != identifierMap.end(); it++)
      {
         CString value = prop.GetParameterValue(L"_identifier_"+CString(identifierCount++));
         it->second = value.GetAsciiString();
      }

      // loop over all refs and replace
      for(LONG i=0;i<refs.GetCount();i++)
      {
         ICECompoundNode tree(refs[i]);
         if(tree.IsValid())
         {
            for(LONG j=0;j<paramName.GetCount();j++)
            {
               ICENodeInputPort port = tree.GetInputPorts().GetItem(paramName[j]);
               Parameter param = port.GetParameters()[0];
               CString value = CString(param.GetValue()).GetAsciiString();
               stringMapIt it = maps[j]->find(value.GetAsciiString());
               if(it != maps[j]->end())
               {
                  if(CString(it->second.c_str()) != value)
                  {
                     if(j<2)
                        addRefArchive(CString(it->second.c_str()));
                     param.PutValue(CString(it->second.c_str()));
                  }
               }
            }
            continue;
         }
         CustomOperator op(refs[i]);
         if(op.IsValid())
         {
            for(LONG j=0;j<paramName.GetCount();j++)
            {
               CString value = op.GetParameterValue(paramName[j]);
               stringMapIt it = maps[j]->find(value.GetAsciiString());
               if(it != maps[j]->end())
               {
                  if(CString(it->second.c_str()) != value)
                  {
                     if(j<2)
                        addRefArchive(CString(it->second.c_str()));
                     op.PutParameterValue(paramName[j],CString(it->second.c_str()));
                  }
               }
            }
            continue;
         }
      }
   }

   // if we still have the property, we need to delete it!
   if(prop.IsValid())
   {
      CValue returnVal;
      CValueArray cmdArgs;
      cmdArgs.Add(prop.GetFullName());
      Application().ExecuteCommand(L"DeleteObj",cmdArgs,returnVal);
   }

   return CStatus::OK;
ESS_CALLBACK_END