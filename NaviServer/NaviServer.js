// Express.js 설정
const express = require('express');
const { json } = require('express/lib/response');
const app = express();

// Json Parser 설정
const bodyParser = require('body-parser');
app.use(bodyParser.urlencoded({extended:false}));
app.use(bodyParser.json());
app.use(express.static(__dirname+'/'));

// DB 설정
var mysql = require('mysql');
var pool = mysql.createPool({
    connectionLimit: 100,
    host: 'localhost',
    port: 3306,
    user: 'root',
    password: '123456789',
    database: 'navidb'
});

//-----------------------------------------------------------------------------------------------

const server = app.listen(3000, ()=>{
    console.log("Start Server : localhost:3000");
});

// 서버 연결 테스트 
app.get('/connection-test', function(req, res){
    res.send('Hello Navi Client');
});

// 캐릭터 정보 조회 
app.get('/character', function(req, res){
    var user_id = req.query.user_id;
    var result_object = new Object();
    
     pool.getConnection(function(err, db){
         if(err) {
             result_object.code = 500;
             db.release();
             var result_json = JSON.stringify(result_object) ;
             res.send(result_json);
             throw err;
         } 
 
         var select_param = [user_id]; 
         db.query('SELECT json_extract(character_info, "$.level", "$.exp", "$.skill_point", "$.skill_one_level", "$.skill_two_level") FROM navi_character WHERE user_id = ?', select_param, function(err, result){     
             if(result.length == 1) {
                var json_extr = result[0]["json_extract(character_info, \"$.level\", \"$.exp\", \"$.skill_point\", \"$.skill_one_level\", \"$.skill_two_level\")"];

                var charinfo = JSON.parse(json_extr);
                result_object.code = 200;
                result_object.level = charinfo[0];
                result_object.exp = charinfo[1];
                result_object.skill_point = charinfo[2];
                result_object.skill_one_level = charinfo[3];
                result_object.skill_two_level = charinfo[4];

                db.release();
                var result_json = JSON.stringify(result_object) ;
                res.send(result_json);
             }
             else {
                result_object.code = 500;
                db.release();
                var result_json = JSON.stringify(result_object) ;
                res.send(result_json);
             }
         });
     });
});


// 회원가입 요청 
app.post('/user/signup', function(req, res, rec){
    const JsonObj = JSON.parse(JSON.stringify(req.body));
    var signup_id = JsonObj.signup_id;
    var signup_pw = JsonObj.signup_pw;

    var result_object = new Object(); 

    pool.getConnection(function(err, db){
        if(err){
            result_object.code = 500;
            var result_json = JSON.stringify(result_object) ;
            db.release();
            res.send(result_json);
            throw err;
        } 

        // ID 중복 검사
        var select_param = [signup_id];
        db.query('SELECT user_id FROM navi_user WHERE user_id = ?', select_param, function(err, result){     
            if(result.length >= 1){
                result_object.code = 409;
                db.release();
                var result_json = JSON.stringify(result_object);
                res.send(result_json);      
            }
            else{
                var insert_param = [signup_id, signup_pw];
                db.query('INSERT INTO navi_user(user_id, password) VALUE(?, ?)', insert_param, function(err, result){
                    if(result.affectedRows > 0){
                        result_object.code = 200;
                        db.release();
                        var result_json = JSON.stringify(result_object);
                        res.send(result_json);  
                    }
                    else {
                        result_object.code = 500;
                        db.release();
                        var result_json = JSON.stringify(result_object);
                        res.send(result_json);  
                    }
                });
            }
        }); 
    });
});

// 로그인 요청
app.post('/user/login', function(req, res, rec){
    const JsonObj = JSON.parse(JSON.stringify(req.body));
    var login_id = JsonObj.login_id;
    var login_pw = JsonObj.login_pw;

    var result_object = new Object(); 

    pool.getConnection(function(err, db){
        if(err){
            result_object.code = 500;
            var result_json = JSON.stringify(result_object) ;
            res.send(result_json);
            db.release();
            throw err;
        } 
        
        // 해당 ID가 존재하는지 확인
        var select_param = [login_id];
        db.query('SELECT user_id, password FROM navi_user WHERE user_id = ?', select_param, function(err, result){     
            if(result.length == 0){
                result_object.code = 404;
                var result_json = JSON.stringify(result_object) ;
                res.send(result_json);
                db.release();
            }
            else if(result.length == 1) {
                var db_id = result[0].user_id;
                var db_pw = result[0].password;

                if (login_id == db_id){
                    if (login_pw == db_pw){// 입력된 ID/PW와 DB의 ID/PW가 일치
                        result_object.code = 200;
                        db.release();
                        var result_json = JSON.stringify(result_object) ;
                        res.send(result_json);
                    }
                    else{// ID는 DB에 존재하나 PW가 불일치
                        result_object.code = 401;
                        db.release();
                        var result_json = JSON.stringify(result_object) ;
                        res.send(result_json);
                    }
                } 
            }
            else{// 쿼리 결과가 여러 개라면 잘못된 작동 
                result_object.code = 500;
                db.release();
                var result_json = JSON.stringify(result_object) ;
                res.send(result_json);
                throw err;
            }
            
        });
    });
});

// 캐릭터 생성 요청 
app.post('/character', function(req, res, rec){
    const JsonObj = JSON.parse(JSON.stringify(req.body));
    var character_id = JsonObj.character_id;
    var user_id = JsonObj.user_id;

    var result_object = new Object(); 

    pool.getConnection(function(err, db){
        if(err){
            result_object.code = 500;
            db.release();
            var result_json = JSON.stringify(result_object) ;
            res.send(result_json);
            throw err;
        }
        
        var insert_param = [character_id, user_id];
        db.query('INSERT INTO navi_character(character_id, user_id) VALUE(?, ?)', insert_param, function(err, result){
            if(result.affectedRows > 0){
                result_object.code = 200;
                db.release();
                var result_json = JSON.stringify(result_object) ;
                res.send(result_json);
            }
            else{
                result_object.code = 500;
                db.release();
                var result_json = JSON.stringify(result_object) ;
                res.send(result_json);
            }
        });
    });
});

// 스탯 업로드
app.put('/character', function(req, res, rec){
    const JsonObj = JSON.parse(JSON.stringify(req.body));
    var character_id = JsonObj.character_id;
    var level = JsonObj.level;
    var exp = JsonObj.exp;
    var skill_point = JsonObj.skill_point;
    var skill_one_level = JsonObj.skill_one_level;
    var skill_two_level = JsonObj.skill_two_level;

    var result_object = new Object(); 

    pool.getConnection(function(err, db){
        if(err){
            result_object.code = 500;
            var result_json = JSON.stringify(result_object) ;
            res.send(result_json);
            db.release();
            throw err;
        } 

        var sql = `UPDATE navi_character SET character_info = '{ "level" : ${level}, "exp" : ${exp}, "skill_point" : ${skill_point}, "skill_one_level" : ${skill_one_level}, "skill_two_level" : ${skill_two_level} }' WHERE character_id = '${character_id}';`;
        db.query(sql, function(err, result){
            if(result.affectedRows > 0){
                result_object.code = 200;
                db.release();
                var result_json = JSON.stringify(result_object) ;
                res.send(result_json);
            }
            else{
                result_object.code = 500;
                db.release();
                var result_json = JSON.stringify(result_object) ;
                res.send(result_json);
            }
        });
    });
});

function ToItemStateEnum(number){
    if(number == 0){
        return 'NoHave';
    }
    else if (number == 1){
        return 'Have';
    }
    else if(number == 2){
        return 'Equipped';
    }
}

// 인벤토리 아이템 추가 
app.put('/inventory', function(req, res, rec){
    const JsonObj = JSON.parse(JSON.stringify(req.body));
    var character_id = JsonObj.character_id;
    var smg_common = ToItemStateEnum(JsonObj.smg_common);
    var smg_rare = ToItemStateEnum(JsonObj.smg_rare)
    var smg_legendary = ToItemStateEnum(JsonObj.smg_legendary);
    var ar_common = ToItemStateEnum(JsonObj.ar_common);
    var ar_rare = ToItemStateEnum(JsonObj.ar_rare);
    var ar_legendary = ToItemStateEnum(JsonObj.ar_legendary);
    var pis_common = ToItemStateEnum(JsonObj.pis_common);
    var pis_rare = ToItemStateEnum(JsonObj.pis_rare);
    var pis_legendary = ToItemStateEnum(JsonObj.pis_legendary);
    var ring_rare = ToItemStateEnum(JsonObj.ring_rare);
    var ring_legendary = ToItemStateEnum(JsonObj.ring_legendary);
    var necklace_rare = ToItemStateEnum(JsonObj.necklace_rare);
    var necklace_legendary = ToItemStateEnum(JsonObj.necklace_legendary);

    var result_object = new Object(); 

    pool.getConnection(function(err, db){
        if(err){
            result_object.code = 500;
            var result_json = JSON.stringify(result_object) ;
            res.send(result_json);
            db.release();
            throw err;
        }
        
        var sql = `UPDATE navi_character SET inventory_info = '{ "smg_common" : "${smg_common}", "smg_rare" : "${smg_rare}", "smg_legendary" : "${smg_legendary}", "ar_common" : "${ar_common}", "ar_rare" : "${ar_rare}", "ar_legendary" : "${ar_legendary}", "pis_common" : "${pis_common}", "pis_rare" : "${pis_rare}", "pis_legendary" : "${pis_legendary}", "ring_rare" : "${ring_rare}", "ring_legendary" : "${ring_legendary}", "necklace_rare" : "${necklace_rare}", "necklace_legendary" : "${necklace_legendary}"}' WHERE character_id = '${character_id}';`;
        var update_param = [smg_common, smg_rare, smg_legendary, ar_common, ar_rare, ar_legendary, pis_common, pis_rare, pis_legendary, necklace_rare, necklace_legendary, ring_rare, ring_legendary, character_id];
        db.query(sql, function(err, result){
            if(result.affectedRows > 0){
                result_object.code = 200;
                db.release();
                var result_json = JSON.stringify(result_object) ;
                res.send(result_json);
            }
            else{
                result_object.code = 500;
                db.release();
                var result_json = JSON.stringify(result_object) ;
                res.send(result_json);
            }
        });
    });
});

// 인벤토리 정보 조회
app.get('/inventory', function(req, res){
    var character_id = req.query.character_id;
    var result_object = new Object();

    pool.getConnection(function(err, db){
        if(err){
            result_object.code = 500;
            var result_json = JSON.stringify(result_object) ;
            res.send(result_json);
            db.release();
            throw err;
        } 
 
        var sql = `SELECT json_extract(inventory_info, "$.smg_common", "$.smg_rare", "$.smg_legendary", "$.ar_common", "$.ar_rare", "$.ar_legendary", "$.pis_common", "$.pis_rare", "$.pis_legendary", "$.ring_rare", "$.ring_legendary", "$.necklace_rare", "$.necklace_legendary") FROM navi_character WHERE character_id = '${character_id}'`;
        db.query(sql, function(err, result){ 
            if(result.length == 1) {
               var json_extr = result[0]["json_extract(inventory_info, \"$.smg_common\", \"$.smg_rare\", \"$.smg_legendary\", \"$.ar_common\", \"$.ar_rare\", \"$.ar_legendary\", \"$.pis_common\", \"$.pis_rare\", \"$.pis_legendary\", \"$.ring_rare\", \"$.ring_legendary\", \"$.necklace_rare\", \"$.necklace_legendary\")"];
               var invinfo = JSON.parse(json_extr);

               result_object.code = 200;
               result_object.smg_common = invinfo[0];
               result_object.smg_rare = invinfo[1];
               result_object.smg_legendary = invinfo[2];
               result_object.ar_common = invinfo[3];
               result_object.ar_rare = invinfo[4];
               result_object.ar_legendary = invinfo[5];
               result_object.pis_common = invinfo[6];
               result_object.pis_rare = invinfo[7];
               result_object.pis_legendary = invinfo[8];
               result_object.ring_rare = invinfo[9];
               result_object.ring_legendary = invinfo[10];
               result_object.necklace_rare = invinfo[11];
               result_object.necklace_legendary = invinfo[12];

               db.release();
               var result_json = JSON.stringify(result_object) ;
               res.send(result_json);
            }
            else{
               result_object.code = 500;
               db.release();
               var result_json = JSON.stringify(result_object) ;
               res.send(result_json);
            }
        });
    });
});
