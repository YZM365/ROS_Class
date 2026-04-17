#!/usr/bin/env python3
from openai import OpenAI
import rospy
from std_msgs.msg import String
import threading  # 新增：解决多次调用阻塞问题

api_key = "sk-Zr00xZdomdsH5o8OVK4MAPQk9jHYQLVt9NijwYD3373HTuYU"
base_url = "https://api.moonshot.cn/v1"

class LLM(OpenAI):
    def __init__(self):

        super().__init__(api_key=api_key,
                         base_url=base_url)
        
        self.model = "moonshot-v1-8k"
        self.system_role_content = "你是Kimi,由 Moonshot AI 提供的人工智能助手, \
        我们将会叫你的小名“小月”,你不会在你的回答中提及你的小名,你更擅长中文和英文的对话. \
        你会为用户提供安全,有帮助,准确的回答. \
        同时,你会拒绝一切涉及恐怖主义,种族歧视,黄色暴力等问题的回答"

        rospy.init_node('robot_voice_llm_node', anonymous=True)
        rospy.Subscriber("/speech/result", String, self.speech_result_callback)
        
        # 新增：创建文字转语音发布者，严格匹配你指定的话题 /talk
        self.tts_pub = rospy.Publisher("/talk", String, queue_size=10)

    def speech_result_callback(self, msg):
        result = msg.data
        print("speech [{}]".format(result))
        if result:
            # 核心修复：子线程处理请求，不阻塞ROS，支持无限次调用
            threading.Thread(target=self.process_llm, args=(result,), daemon=True).start()

    # 新增：子线程处理LLM调用+语音发布
    def process_llm(self, result):
        try:
            chat_response = self.query(result)
            indented_response = "\n".join(f"\t{line}" for line in chat_response.splitlines())
            print(f"LLM的返回结果: \n\n'''\n{indented_response}\n'''")
            
            # 核心功能：向 /talk 话题发布回答，实现文字转语音
            # 等价于命令：rostopic pub -1 /talk std_msgs/String "回答内容"
            self.tts_pub.publish(chat_response)

        except Exception as e:
            if "rate_limit_reached" in str(e):
                print("请求超限")
            else:
                print("出错啦")
    
    def get_system_role_prompt(self):
        return {"role":"system",  "content": self.system_role_content}

    def user_prompt(self, user_prompt):
        return {"role": "user", "content": user_prompt}

    def query(self, user_prompt):
        user_message = [self.get_system_role_prompt(), self.user_prompt(user_prompt)]
        completion = self.chat.completions.create(
            model=self.model,
            messages=user_message,
            temperature=0.1,
            stream=False
        )
        return completion.choices[0].message.content

if __name__ == "__main__":
    try:
        llm = LLM()
        rospy.spin()  # 修复：删除无效代码，保持节点运行
    except KeyboardInterrupt:
        print("\nCaught Ctrl + C. Exiting")
