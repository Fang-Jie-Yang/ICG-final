from time import sleep
import flask
import os
from flask import Flask, Response, send_from_directory
from flask_cors import CORS, cross_origin
from flask import request
from flask import  send_file

app = Flask(__name__, static_folder='B08902087_hw1', static_url_path='')
CORS(app)

@app.route('/')
@cross_origin()
def serve():
    return send_from_directory(app.static_folder, 'index.html')

@app.route('/render',  methods = ['POST'])
@cross_origin()
def render():
    if request.is_json:
        json_data = request.get_json()
        renderData = json_data['renderData']
        samples_per_pixel = json_data['samples_per_pixel']
        num_item = len(renderData)

        for i in range(num_item):
            mvMat =  renderData[i]['mvMatrix']
            norm_mat =  renderData[i]['mvNormalMatrix']

            with open(f"mv_mat_{i}.txt", 'w') as f:
                for j in range(0, 4):
                    f.write(f"{mvMat[j]} {mvMat[j + 4]} {mvMat[j + 8]} {mvMat[j + 12]}\n")
            
            with open(f"norm_mat_{i}.txt", 'w') as f:
                for j in range(0, 3):
                    f.write(f"{norm_mat[j]} {norm_mat[j + 3]} {norm_mat[j + 6]}\n")
                    
        pos_mat_norm_mat_material = ' '.join([ f"mv_mat_{i}.txt norm_mat_{i}.txt {renderData[i]['meterial']}" for i in range(num_item)])
        res = os.system(f"./a.out {samples_per_pixel} {num_item} {pos_mat_norm_mat_material} > out.ppm")

        print(res)
        print(f"./a.out {samples_per_pixel} {num_item} {pos_mat_norm_mat_material} > out.ppm")
        
        if res == 0:
            return send_file('./out.ppm')
        else:
            return Response(status=400)
    else:
        return  Response(status=400)

if __name__ == '__main__':
    app.run()