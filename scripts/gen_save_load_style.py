import csv

print('----------- STYLE SAVE  -------------------')
with open('style.csv', newline='') as csvfile:
    reader = csv.reader(csvfile, delimiter=',', quotechar='"')
    print('auto &style = ImGui::GetStyle();')
    print('YAML::Emitter out;')
    print('out << YAML::BeginMap;')
    print('{')
    print('    out << YAML:: Key << "style";')
    print('    out << YAML::BeginMap;')
    print('    {')
    for row in reader:
        type = row[0]
        field = row[1]
        comment = row[2]
        print('      out << YAML::Key << "' + field + '";')
        if type == 'ImVec2':
            print('      out << YAML::Flow << YAML::BeginSeq << style.' + field + '.x << style.' + field + '.y << YAML::EndSeq;')
        else:
            print('      out << YAML::Value << style.' + field + ';')
        print('      out << YAML::Comment("' + comment + '");')
    print('    }')
    print('    out << YAML::EndMap;')
    print('}')
    print('out << YAML::EndMap;')


print('----------- STYLE LOAD  -------------------')
with open('style.csv', newline='') as csvfile:
    reader = csv.reader(csvfile, delimiter=',', quotechar='"')
    for row in reader:
        type = row[0]
        field = row[1]
        print('if( style["'+field+'"]) {')
        if type == 'ImVec2':
            print('  ImGui::GetStyle().'+field+' = {')
            print('    style["'+field+'"][0].as<float>(),')
            print('    style["'+field+'"][1].as<float>()')
            print('  };')
        elif type == "float":
            print('ImGui::GetStyle().' + field + ' = style["' + field + '"].as<float>();')
        elif type == "bool":
            print('ImGui::GetStyle().'+field+' = style["'+field+'"].as<bool>();')
        print('}')


print('----------- COLORS SAVE  -------------------')
with open('colors.csv', newline='') as csvfile:
    reader = csv.reader(csvfile, delimiter=',', quotechar='"')
    for row in reader:
        field = row[0]
        comment = row[1]
        print('      out << YAML::Key << "' + field + '";')
        print('      out << YAML::Value << colors['+field+'];')
        if comment != "":
            print('      out << YAML::Comment("' + comment + '");')


print('----------- COLORS LOAD  -------------------')
with open('colors.csv', newline='') as csvfile:
    reader = csv.reader(csvfile, delimiter=',', quotechar='"')
    for row in reader:
        field = row[0]
        print('SetStyleColor("'+field+'", '+field+', colors);')
